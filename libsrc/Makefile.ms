clean all:
	cd evalresp_log && $(MAKE) /F Makefile.ms $*
	cd spline && $(MAKE) /F Makefile.ms $*
	cd mxml && $(MAKE) /F Makefile.ms $*
	cd evalresp && $(MAKE) /F Makefile.ms $*

