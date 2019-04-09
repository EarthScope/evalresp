
# This Makefile requires GNU make, sometimes available as gmake.

CFLAGS += -I.. -I../mxml

MXML_SRC= mxml-attr.c mxml-entity.c mxml-file.c mxml-get.c mxml-index.c\
	mxml-node.c mxml-private.c mxml-search.c mxml-set.c mxml-string.c\
	mxmldoc.c
MXML_HEADERS= mxml-private.h mxml.h

vpath %.c .
OBJ=$(addprefix $(BUILD_DIR)/, $(patsubst %.c, %.o,$(notdir $(MXML_SRC))))
TARGET=libmxmlev.a
.PHONY: all clean install
.SUFFIXES:
.SUFFIXES: .c .o

all: config.h $(BUILD_DIR)/$(TARGET)

$(BUILD_DIR)/%.o: %.c $(BUILD_DIR) $(BUILD_DIR)
	$(CC) -c -o $@ $(CFLAGS) $<

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/$(TARGET): $(OBJ)
	$(AR) $(ARFLAGS) $@ $^

config.h: config.h.unix
	cp config.h.unix config.h

clean:
	rm -f $(BUILD_DIR)/$(TARGET) $(OBJ)
ifneq ("$(BUILD_DIR)",".")
	rm -rf $(BUILD_DIR)
endif

install: $(BUILD_DIR)/$(TARGET)
	mkdir -p $(LIB_DIR)
	cp $(BUILD_DIR)/$(TARGET) $(LIB_DIR)/$(TARGET)
	mkdir -p $(INCLUDE_DIR)/mxml
	cp $(MXML_HEADERS) $(INCLUDE_DIR)/mxml/
