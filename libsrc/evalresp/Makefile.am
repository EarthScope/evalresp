
lib_LTLIBRARIES = libevalresp.la

libevalresp_la_SOURCES = input.c evaluation.c output.c highlevel.c\
						 regexp.c regerror.c\
						 regsub.c calc_fctns.c\
						 resp_fctns.c file_ops.c\
						 libevalresp.c alloc_fctns.c\
						 spline.c legacy_interface.c\
						 stationxml2resp/dom_to_seed.c\
						 stationxml2resp/xml_to_dom.c\
						 stationxml2resp/wrappers.c

libevalresp_la_CFLAGS = -I../

evalresp_includedir=$(includedir)/evalresp

nobase_evalresp_include_HEADERS = public.h \
								  public_compat.h \
								  public_api.h \
								  public_channels.h \
								  public_responses.h \
								  stationxml2resp.h \
								  evresp.h
