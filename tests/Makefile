# This Makefile requires GNU make, sometimes available as gmake.
#
# Builds and runs the test suite.  Normally run from the top level with
#
#   make check
#
# which builds the libraries and programs first.  The suite needs only a C
# compiler; see README.md.

BUILD_DIR ?= .

LIBSRC = ../libsrc
LIBFILES = $(LIBSRC)/evalresp/$(BUILD_DIR)/libevalresp.a \
           $(LIBSRC)/spline/$(BUILD_DIR)/libspline.a \
           $(LIBSRC)/evalresp_log/$(BUILD_DIR)/libevalresp_log.a \
           $(LIBSRC)/mxml/$(BUILD_DIR)/libmxmlev.a
LDLIBS = $(LIBFILES) -lm

override CFLAGS += -I. -I$(LIBSRC) -I$(LIBSRC)/mxml

HARNESS = harness/harness.c harness/compare.c harness/platform.c
UNIT = $(addprefix unit/test_,$(addsuffix .c,auto compare convert count evaluation globmatch input log match \
        parse_datetime read_xml response response_char spline xml_to_char))
CLI = cli/test_cli.c

SRCS = $(HARNESS) suites.c $(UNIT) $(CLI)
OBJS = $(SRCS:.c=.o)

.PHONY: all check clean
.SUFFIXES:
.SUFFIXES: .c .o

all: run-tests

.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJS): harness/harness.h harness/compare.h harness/platform.h

run-tests: $(OBJS) $(LIBFILES)
	$(CC) -o $@ $(OBJS) $(LDLIBS)

check: run-tests
	./run-tests --data data --expected expected --bin ../src/$(BUILD_DIR) --work work

clean:
	rm -f run-tests $(OBJS)
	rm -rf work run-tests.dSYM
