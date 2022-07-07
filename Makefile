
# This is a GNU Makefile. This should work on most Un*x systems, Windows
# undoubtedly needs some work.

# Package name and version number:
dist = lua-tk-$(version)
version = 1.0

# This has been set up so that it will work with luarocks or without it.
# With luarocks: `sudo luarocks make` to build and install, `sudo luarocks
# remove luatk` to remove the `luatk` rock from your system.
# Without luarocks: `make && sudo make install` to build and install, `sudo
# make uninstall` to remove the `tk.so` module from your system.

# Where to install compiled (C) modules.
installdir = $(shell pkgconf --variable INSTALL_CMOD lua)
# Where to install source (Lua) modules.
installdir_l = $(shell pkgconf --variable INSTALL_LMOD lua)

distfiles = COPYING README.md Makefile tk.c examples/*

# You may have to adjust these to the appropriate values for your system if
# you're installing without luarocks.
LIBFLAG ?= -shared
CFLAGS ?= -O2 -fPIC
INST_LIBDIR ?= $(installdir)

all: tk.so

tk.so: tk.c
	$(CC) $(LIBFLAG) $(CFLAGS) -o $@ $< $(shell pkg-config --cflags --libs tk lua)

clean:
	rm -f tk.so

# Note that only the compiled C module is installed, so you'll have to copy
# the examples and documentation files manually if you want to keep those.

install: tk.so
	mkdir -p $(DESTDIR)$(INST_LIBDIR)
	cp tk.so $(DESTDIR)$(INST_LIBDIR)

uninstall:
	rm -f $(DESTDIR)$(INST_LIBDIR)/tk.so

# Roll a distribution tarball.

dist:
	rm -rf $(dist)
	mkdir $(dist) && mkdir $(dist)/examples
	for x in $(distfiles); do ln -sf $$PWD/$$x $(dist)/$$x; done
	rm -f $(dist).tar.gz
	tar cfzh $(dist).tar.gz $(dist)
	rm -rf $(dist)

distcheck: dist
	tar xfz $(dist).tar.gz
	cd $(dist) && make && make install DESTDIR=./BUILD
	rm -rf $(dist)
