
# This Makefile requires GNU make, sometimes available as gmake.

CFLAGS += -I..

EVALRESP_LOG_SRC= helpers.c log.c examples/to_syslog.c examples/to_file.c
EVALRESP_LOG_HEADERS= log.h examples/to_syslog.h examples/to_file.h

#OBJ=$(patsubst %,$(BUILD_DIR)/%,$(patsubst %.c,%.o,$(EVALRESP_LOG_SRC)))
vpath %.c examples
OBJ=$(addprefix $(BUILD_DIR)/, $(patsubst %.c, %.o,$(notdir $(EVALRESP_LOG_SRC))))
TARGET=libevalresp_log.a
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
	mkdir -p $(INCLUDE_DIR)/evalresp_log
	cp $(EVALRESP_LOG_HEADERS) $(INCLUDE_DIR)/evalresp_log/
