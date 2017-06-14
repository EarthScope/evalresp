clean all:
	cd evalresp_log && $(MAKE) /F Makefile.ms $*
	cd spline && $(MAKE) /F Makefile.ms $*
	cd mxml && $(MAKE) $*
	cd evalresp && $(MAKE) $*

