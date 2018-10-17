
AM_LDFLAGS = -L../../libsrc/evalresp -levalresp\
			 -L../../libsrc/spline -lspline\
			 -L../../libsrc/evalresp_log -levalresp_log\
			 -L../../libsrc/mxml -lmxmlev
AM_CFLAGS=-I../../libsrc 

EXTRA_DIST = data old_fctns.h legacy.h old_print_fctns.c

if USE_CHECK
TESTS = check_read_xml check_convert check_parse_datetime check_response \
	check_count check_auto check_match check_log check_input \
	check_response_char check_evaluation check_xml_to_char\
	check_legacy 
#TESTS = check_input

check_PROGRAMS = check_read_xml check_convert check_parse_datetime check_response \
	check_count check_auto check_match check_log check_input \
	check_response_char check_evaluation check_xml_to_char\
	check_legacy

check_read_xml_SOURCES = check_read_xml.c
check_read_xml_CFLAGS = @CHECK_CFLAGS@ -I../../src/ $(AM_CFLAGS)
check_read_xml_LDADD = @CHECK_LIBS@ $(AM_LDFLAGS)

check_convert_SOURCES = check_convert.c
check_convert_CFLAGS = @CHECK_CFLAGS@ -I../../src/ $(AM_CFLAGS)
check_convert_LDADD = @CHECK_LIBS@ $(AM_LDFLAGS)

check_parse_datetime_SOURCES = check_parse_datetime.c
check_parse_datetime_CFLAGS = @CHECK_CFLAGS@ -I../../src/ $(AM_CFLAGS)
check_parse_datetime_LDADD = @CHECK_LIBS@ $(AM_LDFLAGS)

check_response_SOURCES = check_response.c
check_response_CFLAGS = @CHECK_CFLAGS@ -I../../src/ $(AM_CFLAGS)
check_response_LDADD = @CHECK_LIBS@ $(AM_LDFLAGS)

check_count_SOURCES = check_count.c
check_count_CFLAGS = @CHECK_CFLAGS@ -I../../src/ $(AM_CFLAGS)
check_count_LDADD = @CHECK_LIBS@ $(AM_LDFLAGS)

check_auto_SOURCES = check_auto.c
check_auto_CFLAGS = @CHECK_CFLAGS@ -I../../src/ $(AM_CFLAGS)
check_auto_LDADD = @CHECK_LIBS@ $(AM_LDFLAGS)

check_match_SOURCES = check_match.c
check_match_CFLAGS = @CHECK_CFLAGS@ -I../../src/ $(AM_CFLAGS)
check_match_LDADD = @CHECK_LIBS@ $(AM_LDFLAGS)

check_log_SOURCES = check_log.c
check_log_CFLAGS = @CHECK_CFLAGS@ -I../../src/ -I../../src/log $(AM_CFLAGS)
check_log_LDADD = @CHECK_LIBS@ $(AM_LDFLAGS)

check_input_SOURCES = check_input.c
check_input_CFLAGS = @CHECK_CFLAGS@ -I../../src/ -I../../src/log $(AM_CFLAGS)
check_input_LDADD = @CHECK_LIBS@ $(AM_LDFLAGS)

check_response_char_SOURCES = check_response_char.c
check_response_char_CFLAGS = @CHECK_CFLAGS@ -I../../src/ $(AM_CFLAGS)
check_response_char_LDADD = @CHECK_LIBS@ $(AM_LDFLAGS)

check_xml_to_char_SOURCES = check_xml_to_char.c
check_xml_to_char_CFLAGS = @CHECK_CFLAGS@ -I../../src/ $(AM_CFLAGS)
check_xml_to_char_LDADD = @CHECK_LIBS@ $(AM_LDFLAGS)

check_evaluation_SOURCES = check_evaluation.c
check_evaluation_CFLAGS = @CHECK_CFLAGS@ -I../../src/ $(AM_CFLAGS)
check_evaluation_LDADD = @CHECK_LIBS@ $(AM_LDFLAGS)

check_legacy_SOURCES = check_legacy.c old_parse_fctns.c old_string_fctns.c
check_legacy_CFLAGS = @CHECK_CFLAGS@ -I../../src/ $(AM_CFLAGS)
check_legacy_LDADD = @CHECK_LIBS@ $(AM_LDFLAGS)
endif

clean-local:
	rm -f *.xml *.txt *.o

distclean-local:
	rm -f Makefile.in *~
