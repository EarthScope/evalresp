SUBDIRS = mxml

lib_LTLIBRARIES = libevalresp_log.la libspline.la libevalresp.la

libevalresp_log_la_SOURCES = evalresp_log/log.c evalresp_log/helpers.c\
							 evalresp_log/examples/to_syslog.c\
							 evalresp_log/examples/to_file.c

libspline_la_SOURCES = spline/spline.c
libspline_la_LDFLAGS = -levalresp_log

libevalresp_la_SOURCES = evalresp/regexp.c evalresp/regerror.c\
						 evalresp/regsub.c evalresp/calc_fctns.c\
						 evalresp/resp_fctns.c evalresp/file_ops.c\
						 evalresp/libevalresp.c evalresp/alloc_fctns.c\
						 evalresp/parse_fctns.c evalresp/print_fctns.c\
						 evalresp/string_fctns.c evalresp/error_fctns.c\
						 evalresp/spline.c evalresp/x2r/log.c\
						 evalresp/x2r/ws.c evalresp/x2r/xml.c
libevalresp_la_LDFLAGS= -lspline -levalresp_log


nobase_include_HEADERS = evalresp_log/log.h evalresp_log/examples/to_syslog.h\
						 evalresp_log/examples/to_file.h spline/spline.h\
						 evalresp/evalresp.h

