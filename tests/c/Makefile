
# This Makefile requires GNU make, sometimes available as gmake.
#
# Builds and runs the C unit tests.  Requires the "check" library, which
# is located with pkg-config.  Normally run from the top level with
#
#   make -f Makefile check
#
# which builds the libraries first.  Tests read ./data, so they must run
# from this directory (this Makefile takes care of that).

BUILD_DIR ?= .

CHECK_CFLAGS := $(shell pkg-config --cflags check 2>/dev/null)
CHECK_LIBS := $(shell pkg-config --libs check 2>/dev/null)

CFLAGS += -I../../libsrc $(CHECK_CFLAGS)

LIBSRC = ../../libsrc
LIBFILES = $(LIBSRC)/evalresp/$(BUILD_DIR)/libevalresp.a \
           $(LIBSRC)/spline/$(BUILD_DIR)/libspline.a \
           $(LIBSRC)/evalresp_log/$(BUILD_DIR)/libevalresp_log.a \
           $(LIBSRC)/mxml/$(BUILD_DIR)/libmxmlev.a
LDLIBS = $(LIBFILES) $(CHECK_LIBS) -lm

TESTS = check_read_xml check_convert check_parse_datetime check_response \
        check_count check_auto check_match check_log check_input \
        check_response_char check_evaluation check_xml_to_char \
        check_legacy check_spline

# extra sources for individual tests
EXTRA_check_legacy = old_parse_fctns.c old_string_fctns.c

.PHONY: all check clean
.SUFFIXES:

all: check

$(TESTS): %: %.c $(LIBFILES)
	@if [ -z "$(CHECK_LIBS)" ]; then \
	  echo "ERROR: the 'check' library was not found by pkg-config."; \
	  echo "Install it (e.g. libcheck-dev, check-devel, or 'brew install check')."; \
	  exit 1; \
	fi
	$(CC) -o $@ $(CFLAGS) $< $(EXTRA_$@) $(LDLIBS)

check: $(TESTS)
	@failed=0; \
	for t in $(TESTS); do \
	  echo "RUN  $$t"; \
	  ./$$t || { echo "FAIL $$t"; failed=$$((failed + 1)); }; \
	done; \
	echo "$$failed of $(words $(TESTS)) test programs failed"; \
	test $$failed -eq 0

clean:
	rm -f $(TESTS) *.o *.xml *.txt
	rm -rf *.dSYM
