clean all:
    cd mxml && $(MAKE) $*
    cd evalresp_log && $(MAKE) $*
    cd spline && $(MAKE) $*
    cd evalresp && $(MAKE) $*

