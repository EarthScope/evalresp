clean all:
    cd libsrc && $(MAKE) $*
    cd src && $(MAKE) $*
