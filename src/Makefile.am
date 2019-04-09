
AM_CFLAGS = -I../libsrc
commonLDADD = -L../libsrc/evalresp -levalresp\
			  -L../libsrc/spline -lspline\
			  -L../libsrc/evalresp_log -levalresp_log\
			  -L../libsrc/mxml -lmxmlev


bin_PROGRAMS = evalresp xml2resp

evalresp_SOURCE = evalresp.c
evalresp_LDADD =  $(commonLDADD) -lm

xml2resp_SOURCE = xml2resp.c
xml2resp_LDADD =  $(commonLDADD)

EXTRA_DIST = \
	Makefile.nmake Makefile vcs_getopt.h

distclean-local:
	-rm -f Makefile.in *~
