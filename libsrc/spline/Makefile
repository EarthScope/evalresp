
# This Makefile requires GNU make, sometimes available as gmake.

CFLAGS += -I..

SPLINE_SRC= spline.c
SPLINE_HEADERS= spline.h

vpath %.c .
#OBJ=$(patsubst %,$(BUILD_DIR)/%,$(patsubst %.c,%.o,$(EVALRESP_LOG_SRC)))
OBJ=$(addprefix $(BUILD_DIR)/, $(patsubst %.c, %.o,$(notdir $(SPLINE_SRC))))
TARGET=libspline.a
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
	mkdir -p $(INCLUDE_DIR)/spline
	cp spline.h $(INCLUDE_DIR)/spline/

