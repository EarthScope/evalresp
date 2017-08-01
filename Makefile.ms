clean all:
	cd libsrc && $(MAKE) /F Makefile.ms $*
	cd src && $(MAKE) /F Makefile.ms $*

install-for-robot:
	cd libsrc && $(MAKE) /F Makefile.ms install
	cd src && $(MAKE) /F Makefile.ms install
