
lib_LTLIBRARIES = libevalresp_log.la

libevalresp_log_la_SOURCES = log.c helpers.c examples/to_syslog.c\
							 examples/to_file.c
libevalresp_log_la_CFLAGS = -I../
evalresp_log_includedir=$(includedir)/evalresp_log
nobase_evalresp_log_include_HEADERS =  log.h examples/to_syslog.h\
								examples/to_file.h

EXTRA_DIST = Makefile Makefile.nmake
