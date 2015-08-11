
SUBDIRS = java fortran

EXTRA_DIST = data

if USE_CHECK
TESTS = check_read_xml check_convert check_parse_datetime check_response \
	check_count check_auto check_match
#TESTS = check_match

check_PROGRAMS = check_read_xml check_convert check_parse_datetime check_response \
	check_count check_auto check_match

check_read_xml_SOURCES = check_read_xml.c 
check_read_xml_CFLAGS = @CHECK_CFLAGS@ -I../src/
check_read_xml_LDADD = ../src/libx2r.la @CHECK_LIBS@ -lxml2

check_convert_SOURCES = check_convert.c 
check_convert_CFLAGS = @CHECK_CFLAGS@ -I../src/
check_convert_LDADD = ../src/libx2r.la @CHECK_LIBS@ -lxml2

check_parse_datetime_SOURCES = check_parse_datetime.c 
check_parse_datetime_CFLAGS = @CHECK_CFLAGS@ -I../src/
check_parse_datetime_LDADD = ../src/libx2r.la @CHECK_LIBS@ -lxml2

check_response_SOURCES = check_response.c 
check_response_CFLAGS = @CHECK_CFLAGS@ -I../src/
check_response_LDADD = ../src/libx2r.la ../src/libevresp.la @CHECK_LIBS@ -lxml2 

check_count_SOURCES = check_count.c 
check_count_CFLAGS = @CHECK_CFLAGS@ -I../src/
check_count_LDADD = ../src/libx2r.la ../src/libevresp.la @CHECK_LIBS@ -lxml2 

check_auto_SOURCES = check_auto.c 
check_auto_CFLAGS = @CHECK_CFLAGS@ -I../src/
check_auto_LDADD = ../src/libx2r.la ../src/libevresp.la @CHECK_LIBS@ -lxml2 

check_match_SOURCES = check_match.c 
check_match_CFLAGS = @CHECK_CFLAGS@ -I../src/
check_match_LDADD = ../src/libx2r.la ../src/libevresp.la @CHECK_LIBS@ -lxml2 
endif

distclean-local:
	rm -f Makefile.in *~
