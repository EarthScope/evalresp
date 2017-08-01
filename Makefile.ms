clean all install:
	cd libsrc && $(MAKE) /F Makefile.ms $*
	cd src && $(MAKE) /F Makefile.ms $*
