name = stereowrap
linkname = lib$(name).so
soname = $(linkname).1
lib_so = $(soname).0
bin = $(name)

PREFIX=/usr/local
src = $(wildcard src/*.c)
obj = $(src:.c=.o)

CC = gcc
INSTALL = install
CFLAGS = -Wall -fPIC -g -D_GNU_SOURCE

ifeq ($(shell uname -s), SunOS)
	INSTALL = ginstall
endif

$(lib_so): $(obj)
	$(CC) -o $@ -shared -Wl,-soname,$(soname) $(obj) $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(obj) $(lib_so)

.PHONY: install
install:
	$(INSTALL) -d $(PREFIX)/bin
	$(INSTALL) -d $(PREFIX)/lib
	$(INSTALL) -m 644 $(lib_so) $(PREFIX)/lib/$(lib_so)
	cd $(PREFIX)/lib; rm -f $(linkname); ln -s $(lib_so) $(linkname)
	$(INSTALL) -m 755 $(bin) $(PREFIX)/bin/$(bin)

.PHONY: uninstall
uninstall:
	rm -f $(PREFIX)/lib/$(soname)
	rm -f $(PREFIX)/lib/$(lib_so)
	rm -f $(PREFIX)/lib/$(linkname)
	rm -f $(PREFIX)/bin/$(bin)
