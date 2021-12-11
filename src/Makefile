
# This Makefile requires GNU make, sometimes available as gmake.

LDFLAGS += -L ../libsrc/evalresp_log/$(BUILD_DIR)/ -levalresp_log\
		 -L ../libsrc/evalresp/$(BUILD_DIR)/ -levalresp\
		 -L ../libsrc/spline/$(BUILD_DIR)/ -lspline\
		 -L ../libsrc/mxml/ -lmxmlev\
		 -lm
CFLAGS += -I../libsrc -I../libsrc/mxml -DHAVE_GETOPT_H

evalresp_SOURCES=evalresp.c
xml2resp_SOURCES=xml2resp.c
TARGETS = evalresp xml2resp
_targets = $(addprefix $(BUILD_DIR)/,$(TARGETS))
.PHONY: all clean install

all: clean $(BUILD_DIR) $(_targets)
	echo $(_targets)

$(BUILD_DIR)/evalresp: $(evalresp_SOURCES)
	$(CC) -o $@ $(CFLAGS) $^ $(LDFLAGS)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/xml2resp: $(xml2resp_SOURCES)
	$(CC) -o $@ $(CFLAGS) $^ $(LDFLAGS)

clean:
	rm -f $(_targets)
ifneq ("$(BUILD_DIR)", ".")
	rm -rf $(BUILD_DIR)
endif

install:
	mkdir -p $(BIN_DIR)
	cp $(_targets) $(BIN_DIR)
