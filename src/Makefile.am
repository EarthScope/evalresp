
LDFLAGS+=-L../libsrc/mxml

AM_LDFLAGS=-L../libsrc -levalresp_log -levresp -lx2r -lmxmlev
AM_CFLAGS = -I../libsrc

bin_PROGRAMS = evalresp xml2resp

evalresp_SOURCE = evalresp.c
evalresp_LDADD =  $(AM_LDFLAGS) -lm

xml2resp_SOURCE = xml2resp.c
xml2resp_LDADD = $(AM_LDFLAGS)

lib_LTLIBRARIES = libevresp.la libx2r.la

libevresp_la_SOURCES = regexp.c regerror.c regsub.c calc_fctns.c resp_fctns.c \
 file_ops.c evresp.c alloc_fctns.c parse_fctns.c print_fctns.c string_fctns.c \
 error_fctns.c spline.c \
 evalresp_spline.c
libevresp_la_LDFLAGS=

libx2r_la_SOURCES = x2r_log.c x2r_xml.c x2r_ws.c
libx2r_la_LDFLAGS=

include_HEADERS = \
	evalresp.h

#SUBDIRS = log

noinst_HEADERS = \
	evresp.h \
	evr_spline.h \
	regexp.h \
	regmagic.h \
	spline.h \
	x2r.h \
	x2r_log.h \
	x2r_ws.h \
	x2r_xml.h

EXTRA_DIST = \
	Makefile.ms

distclean-local:
	-rm -f Makefile.in *~
