
bin_PROGRAMS = evalresp

evalresp_SOURCE = evalresp.c
evalresp_LDADD =  -levresp -lm

lib_LTLIBRARIES = libevresp.la

libevresp_la_SOURCES = regexp.c regerror.c regsub.c calc_fctns.c resp_fctns.c \
 file_ops.c evresp.c alloc_fctns.c parse_fctns.c print_fctns.c string_fctns.c \
 error_fctns.c evr_spline.c

include_HEADERS = evresp.h regmagic.h regexp.h evr_spline.h

EXTRA_DIST = evresp.h regmagic.h regexp.h

distclean-local:
	rm -f Makefile.in *~
