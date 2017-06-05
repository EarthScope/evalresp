
LDFLAGS+="-L../libsrc/mxml"

bin_PROGRAMS = evalresp xml2resp

evalresp_SOURCE = evalresp.c
evalresp_LDADD =  -levresp -lm -lx2r -lmxmlev

xml2resp_SOURCE = xml2resp.c
xml2resp_LDADD = -levresp -lx2r -lmxmlev

lib_LTLIBRARIES = libevresp.la libx2r.la

libevresp_la_SOURCES = regexp.c regerror.c regsub.c calc_fctns.c resp_fctns.c \
 file_ops.c evresp.c alloc_fctns.c parse_fctns.c print_fctns.c string_fctns.c \
 error_fctns.c spline.c log.c log/to_syslog.c log/to_file.c log/helpers.c \
 evalresp_spline.c

libx2r_la_SOURCES = x2r_log.c x2r_xml.c x2r_ws.c

include_HEADERS = \
	evalresp.h

#SUBDIRS = log

noinst_HEADERS = \
	evresp.h \
	evr_spline.h \
	log.h \
	log/to_file.h \
	log/to_syslog.h \
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
