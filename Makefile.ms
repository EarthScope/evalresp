clean all:
	cd libsrc && $(MAKE) /F Makefile.ms $*
	cd src && $(MAKE) /F Makefile.ms $*