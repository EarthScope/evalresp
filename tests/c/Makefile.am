
LDFLAGS+="-L../../libsrc/mxml"
AM_LDFLAGS=-L../../libsrc -levalresp_log -levresp -lx2r -lmxmlev
AM_CFLAGS=-I../../libsrc 

EXTRA_DIST = data

if USE_CHECK
TESTS = check_read_xml check_convert check_parse_datetime check_response \
	check_count check_auto check_match check_log
#TESTS = check_match

check_PROGRAMS = check_read_xml check_convert check_parse_datetime check_response \
	check_count check_auto check_match check_log

check_read_xml_SOURCES = check_read_xml.c
check_read_xml_CFLAGS = @CHECK_CFLAGS@ -I../../src/ $(AM_CFLAGS)
check_read_xml_LDADD = ../../src/libx2r.la ../../src/libevresp.la @CHECK_LIBS@ $(AM_LDFLAGS)

check_convert_SOURCES = check_convert.c
check_convert_CFLAGS = @CHECK_CFLAGS@ -I../../src/ $(AM_CFLAGS)
check_convert_LDADD = ../../src/libx2r.la ../../src/libevresp.la @CHECK_LIBS@ $(AM_LDFLAGS)

check_parse_datetime_SOURCES = check_parse_datetime.c
check_parse_datetime_CFLAGS = @CHECK_CFLAGS@ -I../../src/ $(AM_CFLAGS)
check_parse_datetime_LDADD = ../../src/libx2r.la ../../src/libevresp.la @CHECK_LIBS@ $(AM_LDFLAGS)

check_response_SOURCES = check_response.c
check_response_CFLAGS = @CHECK_CFLAGS@ -I../../src/ $(AM_CFLAGS)
check_response_LDADD = ../../src/libx2r.la ../../src/libevresp.la @CHECK_LIBS@ $(AM_LDFLAGS)

check_count_SOURCES = check_count.c
check_count_CFLAGS = @CHECK_CFLAGS@ -I../../src/ $(AM_CFLAGS)
check_count_LDADD = ../../src/libx2r.la ../../src/libevresp.la @CHECK_LIBS@ $(AM_LDFLAGS)

check_auto_SOURCES = check_auto.c
check_auto_CFLAGS = @CHECK_CFLAGS@ -I../../src/ $(AM_CFLAGS)
check_auto_LDADD = ../../src/libx2r.la ../../src/libevresp.la @CHECK_LIBS@ $(AM_LDFLAGS)

check_match_SOURCES = check_match.c
check_match_CFLAGS = @CHECK_CFLAGS@ -I../../src/ $(AM_CFLAGS)
check_match_LDADD = ../../src/libx2r.la ../../src/libevresp.la @CHECK_LIBS@ $(AM_LDFLAGS)

check_log_SOURCES = check_log.c
check_log_CFLAGS = @CHECK_CFLAGS@ -I../../src/ -I../../src/log $(AM_CFLAGS)
check_log_LDADD = ../../src/libx2r.la ../../src/libevresp.la @CHECK_LIBS@ $(AM_LDFLAGS)
endif

clean-local:
	rm -f *.xml *.txt *.o

distclean-local:
	rm -f Makefile.in *~
