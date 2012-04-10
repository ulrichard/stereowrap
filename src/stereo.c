/*
stereowrap - an OpenGL stereoscopic emulation layer.
Copyright (C) 2010 - 2012 John Tsiombikas <nuclear@member.fsf.org>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <dlfcn.h>
#include <pwd.h>
#include <termios.h>
#include <fcntl.h>
#include <X11/Xlib.h>
#include <GL/gl.h>
#include <GL/glx.h>

#define DEBUG \
	do { \
		if(debug) \
			printf("STEREOWRAP_DEBUG: %s called\n", __func__); \
	} while(0)

enum {
	PARALLEL,
	CROSS,
	REDBLUE,
	REDCYAN,
	GREENMAG,
	COLORCODE,
	SEQUENTIAL
};

#define LEFT_TEX	rtex[swap_eyes ? 1 : 0]
#define RIGHT_TEX	rtex[swap_eyes ? 0 : 1]
#define USE_SDR		(glCreateProgramObjectARB != 0 && use_shaders)

static int init(void);
static int readcfg(void);
static int parse_method(const char *name);
static int init_textures(void);
static int init_ext(void);
void glDrawBuffer(GLenum buf);
void glXSwapBuffers(Display *dpy, GLXDrawable drawable);
XVisualInfo *glXChooseVisual(Display *dpy, int scr, int *attr);
GLXFBConfig *glXChooseFBConfig(Display *dpy, int scr, const int *attr, int *nitems);
static void show_stereo_pair(void);
static void draw_quad(float x1, float y1, float x2, float y2);
static unsigned int create_pixel_shader(const char *src);
static void show_parallel(void);
static void show_cross(void);
static void show_redblue(void);
static void show_redcyan(void);
static void show_greenmag(void);
static void show_colorcode(void);
static void show_sequential(void);
static void sdr_combine(const char *sdr);

static void (*draw_buffer)(GLenum);
static void (*swap_buffers)(Display*, GLXDrawable);
static XVisualInfo *(*choose_visual)(Display*, int, int*);
static GLXFBConfig *(*choose_fbconfig)(Display*, int, const int*, int*);

static int xsz, ysz;
static unsigned int rtex[2];
static int cur_buf = -1;
static int swap_eyes;
static int stereo_method;
static int grey;
static int use_shaders = 1;		/* if available */
static int debug;
static FILE* serial_port;

static Display *dpy;
static GLXDrawable drawable;

#ifdef GL_ARB_shader_objects
static PFNGLCREATEPROGRAMOBJECTARBPROC glCreateProgramObjectARB;
static PFNGLCREATESHADEROBJECTARBPROC glCreateShaderObjectARB;
static PFNGLDELETEOBJECTARBPROC glDeleteObjectARB;
static PFNGLATTACHOBJECTARBPROC glAttachObjectARB;
static PFNGLSHADERSOURCEARBPROC glShaderSourceARB;
static PFNGLCOMPILESHADERARBPROC glCompileShaderARB;
static PFNGLLINKPROGRAMARBPROC glLinkProgramARB;
static PFNGLUSEPROGRAMOBJECTARBPROC glUseProgramObjectARB;
static PFNGLGETOBJECTPARAMETERIVARBPROC glGetObjectParameterivARB;
static PFNGLGETINFOLOGARBPROC glGetInfoLogARB;
static PFNGLGETUNIFORMLOCATIONARBPROC glGetUniformLocationARB;
static PFNGLUNIFORM1IARBPROC glUniform1iARB;

/*static PFNGLACTIVETEXTUREARBPROC glActiveTextureARB;*/

#endif	/* GL_ARB_shader_objects */

#ifdef GLX_SGI_swap_control
static PFNGLXSWAPINTERVALSGIPROC glXSwapIntervalSGI;
#endif
#ifdef GLX_EXT_swap_control
static PFNGLXSWAPINTERVALEXTPROC glXSwapIntervalEXT;
#endif


static struct {
	int id;
	const char *name;
	int need_sdr;
	void (*func)(void);
} method[] = {
	{PARALLEL, "parallel", 0, show_parallel},
	{CROSS, "cross", 0, show_cross},
	{REDBLUE, "redblue", 0, show_redblue},
	{REDCYAN, "redcyan", 0, show_redcyan},
	{GREENMAG, "greenmagenta", 0, show_greenmag},
#ifndef NOCOLORCODE
	{COLORCODE, "colorcode", 1, show_colorcode},
#endif
	{SEQUENTIAL, "sequential", 0, show_sequential},
	{0, 0, 0, 0}
};


static int init(void)
{
	static int init_done;
	char *env = 0;

	if(init_done) return 0;

	readcfg();

	draw_buffer = dlsym(RTLD_NEXT, "glDrawBuffer");
	swap_buffers = dlsym(RTLD_NEXT, "glXSwapBuffers");
	choose_visual = dlsym(RTLD_NEXT, "glXChooseVisual");
	choose_fbconfig = dlsym(RTLD_NEXT, "glXChooseFBConfig");


	if(!draw_buffer || !swap_buffers || !choose_visual) {
		fprintf(stderr, "stereowrap: failed to load GL/GLX functions\n");
		return -1;
	}

	init_ext();

	if(getenv("STEREO_SWAP")) {
		swap_eyes = 1;
	}

	if(getenv("STEREO_GREY")) {
		grey = 1;
	}

	if(getenv("STEREO_NOSDR")) {
		use_shaders = 0;
	}

	if((env = getenv("STEREO_METHOD"))) {
		int idx = parse_method(env);
		/* only select a shader-based method if we have shaders */
		if(idx >= 0 && (!method[idx].need_sdr || USE_SDR)) {
			stereo_method = idx;
		}
	}

	/* method-specific init */
	if(stereo_method == SEQUENTIAL) {
#ifdef GLX_SGI_swap_control
		if(glXSwapIntervalSGI) {
			glXSwapIntervalSGI(1);	/* enable v-sync */
		}
#endif
		/* XXX glXSwapIntervalEXT requires dpy and drawable parameters
		 * so we call it in swap_buffers below.
		 */

		// open the serial port for controlling the shutter glasses
		serial_port = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY /*| O_NONBLOCK*/); // todo : make the port configurable
		if(serial_port <= 0)
			fprintf(stderr, "stereowrap: failed to open serial port\n");
		else
		{
			struct termios oldtio, newtio;       //place for old and new port settings for serial port
			tcgetattr(serial_port, &oldtio); // save current port settings
			memset (&newtio, 0, sizeof newtio);

	 
		  	newtio.c_cflag = B38400 | CS8 | CLOCAL | CWRITE;
		  	newtio.c_iflag = IGNPAR;
		  	newtio.c_oflag = 0;
		  	newtio.c_lflag = 0;       //ICANON;
		  	newtio.c_cc[VMIN]=1;
		  	newtio.c_cc[VTIME]=0;
		  	tcflush(serial_port, TCIFLUSH);
		  	tcsetattr(serial_port,TCSANOW,&newtio);

			fprintf(stdout, "stereowrap: serial port opened\n");
		}
	}

	if(getenv("STEREOWRAP_DEBUG")) {
		debug = 1;
	}

	init_done = 1;
	return 0;
}

#define CFGFILE		".stereowrap.conf"
static int readcfg(void)
{
	char *home, buf[512];
	FILE *fp;
	struct passwd *pw;

	if((pw = getpwuid(getuid()))) {
		home = pw->pw_dir;
	} else {
		home = getenv("HOME");
	}
	snprintf(buf, sizeof buf, "%s/%s", home, CFGFILE);

	if(!(fp = fopen(buf, "r"))) {
		return -1;
	}

	while(fgets(buf, sizeof buf, fp)) {
		char *name, *val;
		int enable = -1;

		if(!(name = strtok(buf, " \t\r\n:=")))
			continue;

		if((val = strtok(0, " \t\r\n:="))) {
			if(strcmp(val, "true") == 0) {
				enable = 1;
			} else if(strcmp(val, "false") == 0) {
				enable = 0;
			}
		} else {
			enable = 1;
		}

		if(strcmp(name, "method") == 0) {
			int idx = parse_method(val);
			/* only select a shader-based method if we have shaders */
			if(idx >= 0 && (!method[idx].need_sdr || USE_SDR)) {
				stereo_method = idx;
			}
		} else if(strcmp(name, "swap") == 0 && enable != -1) {
			swap_eyes = enable;
		} else if(strcmp(name, "grey") == 0 && enable != -1) {
			grey = enable;
		}
	}

	fclose(fp);
	return 0;
}

static int parse_method(const char *name)
{
	int i;
	for(i=0; method[i].name; i++) {
		if(strcmp(name, method[i].name) == 0) {
			return i;
		}
	}
	return -1;
}

static int init_textures(void)
{
	if(rtex[0] && rtex[1]) {
		int vp[4];

		glGetIntegerv(GL_VIEWPORT, vp);
		if(vp[2] != xsz || vp[3] != ysz) {
			xsz = vp[2];
			ysz = vp[3];

			glBindTexture(GL_TEXTURE_2D, rtex[0]);
			glTexImage2D(GL_TEXTURE_2D, 0, grey ? 1 : 3, xsz, ysz, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

			glBindTexture(GL_TEXTURE_2D, rtex[1]);
			glTexImage2D(GL_TEXTURE_2D, 0, grey ? 1 : 3, xsz, ysz, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
		}
	} else {
		int i;
		glGenTextures(2, rtex);
		for(i=0; i<2; i++) {
			glBindTexture(GL_TEXTURE_2D, rtex[i]);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		}
	}

	return 0;
}

#ifdef GLX_VERSION_1_4
#define get_proc(type, name) \
	(type)glXGetProcAddress((unsigned char*)name)
#else
#define get_proc(type, name) \
	(type)glXGetProcAddressARB((unsigned char*)name)
#endif

static int init_ext(void)
{
#ifdef GL_ARB_shader_objects
	if(glCreateShaderObjectARB) {
		return 0;
	}

	glCreateProgramObjectARB = get_proc(PFNGLCREATEPROGRAMOBJECTARBPROC, "glCreateProgramObjectARB");
	glCreateShaderObjectARB = get_proc(PFNGLCREATESHADEROBJECTARBPROC, "glCreateShaderObjectARB");
	glDeleteObjectARB = get_proc(PFNGLDELETEOBJECTARBPROC, "glDeleteObjectARB");
	glAttachObjectARB = get_proc(PFNGLATTACHOBJECTARBPROC, "glAttachObjectARB");
	glShaderSourceARB = get_proc(PFNGLSHADERSOURCEARBPROC, "glShaderSourceARB");
	glCompileShaderARB = get_proc(PFNGLCOMPILESHADERARBPROC, "glCompileShaderARB");
	glLinkProgramARB = get_proc(PFNGLLINKPROGRAMARBPROC, "glLinkProgramARB");
	glUseProgramObjectARB = get_proc(PFNGLUSEPROGRAMOBJECTARBPROC, "glUseProgramObjectARB");
	glGetObjectParameterivARB = get_proc(PFNGLGETOBJECTPARAMETERIVARBPROC, "glGetObjectParameterivARB");
	glGetInfoLogARB = get_proc(PFNGLGETINFOLOGARBPROC, "glGetInfoLogARB");
	glGetUniformLocationARB = get_proc(PFNGLGETUNIFORMLOCATIONARBPROC, "glGetUniformLocationARB");
	glUniform1iARB = get_proc(PFNGLUNIFORM1IARBPROC, "glUniform1iARB");

	/*glActiveTextureARB = get_proc(PFNGLACTIVETEXTUREARBPROC, "glActiveTextureARB");*/

	if(!glCreateProgramObjectARB) {
		return -1;
	}
#endif	/* GL_ARB_shader_objects */

#ifdef GLX_SGI_swap_control
	glXSwapIntervalSGI = get_proc(PFNGLXSWAPINTERVALSGIPROC, "glXSwapIntervalSGI");
#endif
#ifdef GLX_EXT_swap_control
	glXSwapIntervalEXT = get_proc(PFNGLXSWAPINTERVALEXTPROC, "glXSwapIntervalEXT");
#endif
	return 0;
}

void glDrawBuffer(GLenum buf)
{
	int new_buf = buf - GL_BACK_LEFT;

	if(new_buf == cur_buf) {
		return;
	}

	glPushAttrib(GL_ALL_ATTRIB_BITS);

	if(init() == -1) {
		glPopAttrib();
		return;
	}
	init_textures();

	DEBUG;

	if(cur_buf != -1) {
		glBindTexture(GL_TEXTURE_2D, rtex[cur_buf]);
		glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, xsz, ysz);
	}
	cur_buf = new_buf;

	glPopAttrib();
}

void glXSwapBuffers(Display *_dpy, GLXDrawable _drawable)
{
	static int called_swapint;
	dpy = _dpy;
	drawable = _drawable;

	glPushAttrib(GL_ALL_ATTRIB_BITS);

	if(init() == -1) {
		glPopAttrib();
		return;
	}
	init_textures();

	DEBUG;

#ifdef GLX_EXT_swap_control
	if(!called_swapint) {
		if(glXSwapIntervalEXT) {
			glXSwapIntervalEXT(dpy, drawable, 1);
		}
		called_swapint = 1;
	}
#endif

	if(cur_buf != -1) {
		glBindTexture(GL_TEXTURE_2D, rtex[cur_buf]);
		glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, xsz, ysz);

		show_stereo_pair();
	}
	cur_buf = -1;

	glPopAttrib();

	swap_buffers(dpy, drawable);
}

XVisualInfo *glXChooseVisual(Display *dpy, int scr, int *attr)
{
	int a, *dest, *src;

	if(init() == -1) {
		return 0;
	}
	DEBUG;

	dest = src = attr;
	do {
		a = *src++;
		if(a != GLX_STEREO) {
			*dest++ = a;
		}
	} while(a != None);

	return choose_visual(dpy, scr, attr);
}

/*static const char *attr_name[] = {
	"None",
	"GLX_USE_GL",
	"GLX_BUFFER_SIZE",
	"GLX_LEVEL",
	"GLX_RGBA",
	"GLX_DOUBLEBUFFER",
	"GLX_STEREO",
	"GLX_AUX_BUFFERS",
	"GLX_RED_SIZE",
	"GLX_GREEN_SIZE",
	"GLX_BLUE_SIZE",
	"GLX_ALPHA_SIZE",
	"GLX_DEPTH_SIZE",
	"GLX_STENCIL_SIZE",
	"GLX_ACCUM_RED_SIZE",
	"GLX_ACCUM_GREEN_SIZE",
	"GLX_ACCUM_BLUE_SIZE",
	"GLX_ACCUM_ALPHA_SIZE"
};*/

GLXFBConfig *glXChooseFBConfig(Display *dpy, int scr, const int *attr, int *nitems)
{
	int a, *dest;
	int *src;
	int found_use_gl = 0;	/* hack */
	GLXFBConfig *cfg;

	if(init() == -1 || !choose_fbconfig) {
		return 0;
	}
	DEBUG;

	dest = src = (int*)attr;
	do {
		a = *src++;

		if(a < GLX_AUX_BUFFERS) {
			if(a != GLX_STEREO && !(a == GLX_USE_GL && found_use_gl)) {
				*dest++ = a;
			}

			if(a == GLX_USE_GL) {
				found_use_gl = 1;
			}
		} else {
			*dest++ = a;
			*dest++ = *src++;
		}
	} while(a != None);

	cfg = choose_fbconfig(dpy, scr, attr, nitems);
	return cfg;
}


static void show_stereo_pair(void)
{
	glClear(GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glEnable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_BLEND);
	glDisable(GL_FOG);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glColor3f(1, 1, 1);

	if(USE_SDR) {
		glUseProgramObjectARB(0);
	}

	method[stereo_method].func();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_TEXTURE);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

static const char *redblue_shader =
	"uniform sampler2D left_tex, right_tex;\n"
	"void main()\n"
	"{\n"
	"    vec3 left = texture2D(left_tex, gl_TexCoord[0].st).rgb;\n"
	"    vec3 right = texture2D(right_tex, gl_TexCoord[0].st).rgb;\n"
	"    float red = (left.r + left.g + left.b) / 3.0;\n"
	"    float blue = (right.r + right.g + right.b) / 3.0;\n"
	"    gl_FragColor = vec4(red, 0.0, blue, 1.0);\n"
	"}\n";

static const char *redcyan_shader =
	"uniform sampler2D left_tex, right_tex;\n"
	"void main()\n"
	"{\n"
	"    vec3 left = texture2D(left_tex, gl_TexCoord[0].st).rgb;\n"
	"    vec3 right = texture2D(right_tex, gl_TexCoord[0].st).rgb;\n"
	"    gl_FragColor = vec4(left.r, right.g, right.b, 1.0);\n"
	"    //gl_FragColor = vec4((left.r + left.g + left.b) / 3.0, right.g, right.b, 1.0);\n"
	"}\n";

static const char *greenmag_shader =
	"uniform sampler2D left_tex, right_tex;\n"
	"void main()\n"
	"{\n"
	"    vec3 left = texture2D(left_tex, gl_TexCoord[0].st).rgb;\n"
	"    vec3 right = texture2D(right_tex, gl_TexCoord[0].st).rgb;\n"
	"    gl_FragColor = vec4(right.r, left.g, right.b, 1.0);\n"
	"    //gl_FragColor = vec4(right.r, (left.r + left.g + left.b) / 3.0, right.b, 1.0);\n"
	"}\n";

static const char *colorcode_shader =
	"uniform sampler2D left_tex, right_tex;\n"
	"void main()\n"
	"{\n"
	"    vec3 left = texture2D(left_tex, gl_TexCoord[0].st).rgb;\n"
	"    vec3 right = texture2D(right_tex, gl_TexCoord[0].st).rgb;\n"
	"    vec3 col, coeff = vec3(0.15, 0.15, 0.7);\n"
	"    col.r = left.r; col.g = left.g;\n"
	"    col.b = dot(right, coeff);\n"
	"    gl_FragColor = vec4(col, 1.0);\n"
	"}\n";

static void show_parallel(void)
{
	glBindTexture(GL_TEXTURE_2D, LEFT_TEX);
	draw_quad(-1, -1, 0, 1);
	glBindTexture(GL_TEXTURE_2D, RIGHT_TEX);
	draw_quad(0, -1, 1, 1);
}

static void show_cross(void)
{
	glBindTexture(GL_TEXTURE_2D, LEFT_TEX);
	draw_quad(0, -1, 1, 1);
	glBindTexture(GL_TEXTURE_2D, RIGHT_TEX);
	draw_quad(-1, -1, 0, 1);
}

static void show_redblue(void)
{
	if(USE_SDR) {
		sdr_combine(redblue_shader);
		return;
	}

	glEnable(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, LEFT_TEX);
	glColorMask(1, 0, 0, 1);
	draw_quad(-1, -1, 1, 1);

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);

	glBindTexture(GL_TEXTURE_2D, RIGHT_TEX);
	glColorMask(0, 0, 1, 1);
	draw_quad(-1, -1, 1, 1);

	glColorMask(1, 1, 1, 1);
}

static void show_redcyan(void)
{
	if(USE_SDR) {
		sdr_combine(redcyan_shader);
		return;
	}

	glEnable(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, LEFT_TEX);
	glColorMask(1, 0, 0, 1);
	draw_quad(-1, -1, 1, 1);

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);

	glBindTexture(GL_TEXTURE_2D, RIGHT_TEX);
	glColorMask(0, 1, 1, 1);
	draw_quad(-1, -1, 1, 1);

	glColorMask(1, 1, 1, 1);
}

static void show_greenmag(void)
{
	if(USE_SDR) {
		sdr_combine(greenmag_shader);
		return;
	}

	glEnable(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, LEFT_TEX);
	glColorMask(0, 1, 0, 1);
	draw_quad(-1, -1, 1, 1);

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);

	glBindTexture(GL_TEXTURE_2D, RIGHT_TEX);
	glColorMask(1, 0, 1, 1);
	draw_quad(-1, -1, 1, 1);

	glColorMask(1, 1, 1, 1);
}

static void show_colorcode(void)
{
	sdr_combine(colorcode_shader);
}

static void show_sequential(void)
{
	XEvent xev;
	/* force the application to redraw immediately afterwards to
	 * try and stay in sync with the monitor refresh.
	 */
	xev.type = Expose;
	xev.xexpose.window = drawable;
	xev.xexpose.x = xev.xexpose.y = 0;
	xev.xexpose.width = xsz;
	xev.xexpose.height = ysz;
	xev.xexpose.count = 0;
	XSendEvent(dpy, drawable, False, 0, &xev);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, LEFT_TEX);
	draw_quad(-1, -1, 1, 1);

	if(serial_port)
		fputc((int)'r', serial_port);

	swap_buffers(dpy, drawable);

	glBindTexture(GL_TEXTURE_2D, RIGHT_TEX);
	draw_quad(-1, -1, 1, 1);

	if(serial_port)
		fputc((int)'l', serial_port);

}

static void sdr_combine(const char *sdrsrc)
{
#ifdef GL_ARB_shader_objects
	static int loc_left, loc_right;
	static unsigned int sdr;

	if(!sdr) {
		if((sdr = create_pixel_shader(sdrsrc))) {
			loc_left = glGetUniformLocationARB(sdr, "left_tex");
			loc_right = glGetUniformLocationARB(sdr, "right_tex");
		} else {
			return;
		}
	}

	glUseProgramObjectARB(sdr);
	glUniform1iARB(loc_left, 0);
	glUniform1iARB(loc_right, 1);

	glActiveTexture(GL_TEXTURE1_ARB);
	glBindTexture(GL_TEXTURE_2D, RIGHT_TEX);
	glEnable(GL_TEXTURE_2D);

	glActiveTexture(GL_TEXTURE0_ARB);
	glBindTexture(GL_TEXTURE_2D, LEFT_TEX);
	glEnable(GL_TEXTURE_2D);

	draw_quad(-1, -1, 1, 1);

	glActiveTexture(GL_TEXTURE1_ARB);
	glDisable(GL_TEXTURE_2D);

	glActiveTexture(GL_TEXTURE0_ARB);
	glDisable(GL_TEXTURE_2D);

	glUseProgramObjectARB(0);
#endif	/* GL_ARB_shader_objects */
}

static void draw_quad(float x1, float y1, float x2, float y2)
{
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0); glVertex2f(x1, y1);
	glTexCoord2f(1, 0); glVertex2f(x2, y1);
	glTexCoord2f(1, 1); glVertex2f(x2, y2);
	glTexCoord2f(0, 1); glVertex2f(x1, y2);
	glEnd();
}

static unsigned int create_pixel_shader(const char *src)
{
	unsigned int sdr, prog = 0;
#ifdef GL_ARB_shader_objects
	int success, info_len;
	char *info_str = 0;

	glGetError();

	sdr = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
	glShaderSourceARB(sdr, 1, &src, 0);
	glCompileShaderARB(sdr);

	glGetObjectParameterivARB(sdr, GL_OBJECT_COMPILE_STATUS_ARB, &success);
	glGetObjectParameterivARB(sdr, GL_OBJECT_INFO_LOG_LENGTH_ARB, &info_len);

	if(info_len) {
		if((info_str = malloc(info_len + 1))) {
			glGetInfoLogARB(sdr, info_len, 0, info_str);
			fprintf(stderr, "shader compilation: %s\n", info_str);
			free(info_str);
		}
	}
	if(!success) {
		glDeleteObjectARB(sdr);
		return 0;
	}

	prog = glCreateProgramObjectARB();
	glAttachObjectARB(prog, sdr);
	glLinkProgramARB(prog);

	info_str = 0;
	glGetObjectParameterivARB(prog, GL_OBJECT_LINK_STATUS_ARB, &success);
	glGetObjectParameterivARB(prog, GL_OBJECT_INFO_LOG_LENGTH_ARB, &info_len);

	if(info_len) {
		if((info_str = malloc(info_len + 1))) {
			glGetInfoLogARB(prog, info_len, 0, info_str);
			fprintf(stderr, "shader linking: %s\n", info_str);
			free(info_str);
		}
	}
	if(!success) {
		glDeleteObjectARB(sdr);
		glDeleteObjectARB(prog);
		return 0;
	}

	assert(glGetError() == GL_NO_ERROR);
#endif	/* GL_ARB_shader_objects */
	return prog;
}
