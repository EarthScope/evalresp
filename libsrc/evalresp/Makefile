
# This Makefile requires GNU make, sometimes available as gmake.

CFLAGS += -I.. -I../mxml

EVALRESP_SRC= alloc_fctns.c calc_fctns.c file_ops.c\
			  regexp.c regsub.c resp_fctns.c spline.c input.c\
			  output.c stationxml2resp/wrappers.c\
			  highlevel.c evaluation.c legacy_interface.c\
			  stationxml2resp/dom_to_seed.c stationxml2resp/xml_to_dom.c
EVALRESP_HEADERS= public_api.h public_channels.h public_responses.h public_compat.h stationxml2resp.h evresp.h

#OBJ=$(patsubst %,$(BUILD_DIR)/%,$(patsubst %.c,%.o,$(EVALRESP_LOG_SRC)))
vpath %.c stationxml2resp
OBJ=$(addprefix $(BUILD_DIR)/, $(patsubst %.c, %.o,$(notdir $(EVALRESP_SRC))))
TARGET=libevalresp.a
.PHONY: all clean install
.SUFFIXES:
.SUFFIXES: .c .o

all: $(BUILD_DIR)/$(TARGET)

$(BUILD_DIR)/%.o: %.c $(BUILD_DIR) $(BUILD_DIR)
	$(CC) -c -o $@ $(CFLAGS) $<

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/$(TARGET): $(OBJ)
	$(AR) $(ARFLAGS) $@ $^

clean:
	rm -f $(BUILD_DIR)/$(TARGET) $(OBJ)
ifneq ("$(BUILD_DIR)",".")
	rm -rf $(BUILD_DIR)
endif

install: $(BUILD_DIR)/$(TARGET)
	cp $(BUILD_DIR)/$(TARGET) $(LIB_DIR)/$(TARGET)
	mkdir -p $(INCLUDE_DIR)/evalresp
	cp $(EVALRESP_HEADERS) $(INCLUDE_DIR)/evalresp/
