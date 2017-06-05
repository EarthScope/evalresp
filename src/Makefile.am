
LDFLAGS+=-L../libsrc/mxml

AM_LDFLAGS=-L../libsrc -levalresp -lspline -levalresp_log -lmxmlev
AM_CFLAGS = -I../libsrc

bin_PROGRAMS = evalresp xml2resp

evalresp_SOURCE = evalresp.c
evalresp_LDADD =  $(AM_LDFLAGS) -lm

xml2resp_SOURCE = xml2resp.c

EXTRA_DIST = \
	Makefile.ms

distclean-local:
	-rm -f Makefile.in *~
