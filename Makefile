# simple Makefile for lui. Works for Linux.
#
# Gunnar Zötl <gz@tset.de>, 2016-2017
# Released under MIT/X11 license. See file LICENSE for details.

TARGET=lui.so
DEBUG = -g -DDEBUG

# try some automatic discovery
OS = $(shell uname -s)
LUAVERSION = $(shell lua -e "print(string.match(_VERSION, '%d+%.%d+'))")
LUA_BINDIR = $(shell dirname `which lua`)
LUAROOT = $(shell dirname $(LUA_BINDIR))

CC=gcc
CFLAGS = -fPIC -Wall $(DEBUG)
LUA_INCDIR = $(LUAROOT)/include
LUA_LIBDIR = $(LUAROOT)/lib
LUA = lua

# OS specialities
ifeq ($(OS),Darwin)
LIBFLAG = -bundle -undefined dynamic_lookup -all_load -framework Cocoa
LIBUIOBJDIR=darwin
else
LIBFLAG = -shared
LIBS=$(shell pkg-config gtk+-3.0 --libs) -lm -ldl
LIBUIOBJDIR=unix
endif

# install target locations
INST_DIR = /usr/local
INST_LIBDIR = $(INST_DIR)/lib/lua/$(LUAVERSION)
INST_LUADIR = $(INST_DIR)/share/lua/$(LUAVERSION)

# no user servicable parts below
LIBUI=libui
LIBUI_OBJS=$(LIBUI)/CMakeFiles/libui.dir/common/*.o $(LIBUI)/CMakeFiles/libui.dir/$(LIBUIOBJDIR)/*.o

all: $(TARGET)

$(TARGET): lui.o luad.o $(LIBUI_OBJS)
	$(CC) $(LIBFLAG) -L$(LUA_LIBDIR) -o $@ $< luad.o $(LIBUI_OBJS) $(LIBS)

lui.o: libui lui.c *.inc.c

.c.o:; $(CC) $(CFLAGS) -I$(LIBUI) -I$(LUA_INCDIR) -c -o $@ $<

$(LIBUI_OBJS): libui
	make -C $(LIBUI)

install: all
	mkdir -p $(INST_LIBDIR)
	cp $(TARGET) $(INST_LIBDIR)

doc:
	cat lui.c *.inc.c | lua ./mkdoc "lui Documentation" - > lui.html
.PHONY: doc

clean:
	make -C $(LIBUI) clean
	rm -f *.o *.so lui.html

distclean: clean
	rm -rf libui

libui:
	git clone https://github.com/andlabs/libui
	cd libui && cmake . && make

update-libui:
	cd libui && git pull && cmake . && make
