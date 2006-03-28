## Process this file with automake to create Makefile.in

AUTOMAKE_OPTIONS = foreign

bin_PROGRAMS = evalresp

evalresp_SOURCES = regexp.c regerror.c regsub.c calc_fctns.c resp_fctns.c \
file_ops.c evresp.c evalresp.c alloc_fctns.c parse_fctns.c print_fctns.c \
string_fctns.c error_fctns.c evr_spline.c


lib_LIBRARIES = libevresp.a
libevresp_a_SOURCES = regexp.c regerror.c regsub.c calc_fctns.c resp_fctns.c \
        file_ops.c evresp.c alloc_fctns.c parse_fctns.c print_fctns.c \
        string_fctns.c error_fctns.c evr_spline.c

#CFLAGS =    -Wall -g

include_HEADERS =  evresp.h   regmagic.h  regexp.h  evr_spline.h
man_MANS = evalresp.man

EXTRA_DIST = evalresp.man evresp.h   regmagic.h  regexp.h  \
        evalresp.ide makefile.bor  makefile.ms evtest.f
