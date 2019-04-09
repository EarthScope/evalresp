
# This Makefile requires GNU make, sometimes available as gmake

LIB_SRC_DIR = libsrc
BIN_SRC_DIR = src
include Build.config
ifeq "$(BUILD_DIR)" ""
BUILD_DIR := .
endif
TOPDIR=$(dir $(realpath $(firstword $(MAKEFILE_LIST))))
ARFLAGS=crv

export
.PHONY: all
.PHONY: clean
.PHONY: install

all:
	$(MAKE) -C $(LIB_SRC_DIR) -f Makefile
	$(MAKE) -C $(BIN_SRC_DIR) -f Makefile

clean:
	$(MAKE) -C $(LIB_SRC_DIR) -f Makefile clean
	$(MAKE) -C $(BIN_SRC_DIR) -f Makefile clean

install:
	$(MAKE) -C $(LIB_SRC_DIR) -f Makefile install
	$(MAKE) -C $(BIN_SRC_DIR) -f Makefile install
