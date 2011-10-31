name = stereowrap
linkname = lib$(name).so
soname = $(linkname).1
lib_so = $(soname).0
bin = $(name)

PREFIX=/usr/local
src = $(wildcard src/*.c)
obj = $(src:.c=.o)

CC = gcc
CFLAGS = -Wall -fPIC -g -D_GNU_SOURCE
LDFLAGS = -lpthread

$(lib_so): $(obj)
	$(CC) -o $@ -shared -Wl,-soname,$(soname) $(obj) $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(obj) $(lib_so)

.PHONY: install
install:
	mkdir -p $(PREFIX)/bin $(PREFIX)/lib
	cp $(lib_so) $(PREFIX)/lib/$(lib_so)
	rm -f $(PREFIX)/lib/$(linkname) $(PREFIX)/lib/$(soname)
	ln -s $(PREFIX)/lib/$(lib_so) $(PREFIX)/lib/$(soname)
	ln -s $(PREFIX)/lib/$(soname) $(PREFIX)/lib/$(linkname)
	cp $(bin) $(PREFIX)/bin/$(bin)

.PHONY: uninstall
uninstall:
	rm -f $(PREFIX)/lib/$(soname)
	rm -f $(PREFIX)/lib/$(lib_so)
	rm -f $(PREFIX)/lib/$(linkname)
	rm -f $(PREFIX)/bin/$(bin)
