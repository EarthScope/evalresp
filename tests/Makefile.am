
TESTS = check_read_xml check_convert

check_PROGRAMS = check_read_xml check_convert

check_read_xml_SOURCES = check_read_xml.c 
check_read_xml_CFLAGS = @CHECK_CFLAGS@ -I../src/
check_read_xml_LDADD = ../src/libx2r.la @CHECK_LIBS@ -lxml2

check_convert_SOURCES = check_convert.c 
check_convert_CFLAGS = @CHECK_CFLAGS@ -I../src/
check_convert_LDADD = ../src/libx2r.la @CHECK_LIBS@ -lxml2

distclean-local:
	rm -f Makefile.in *~

