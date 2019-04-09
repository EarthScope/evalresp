
# This Makefile requires GNU make, sometimes available as gmake.

.PHONY: all clean

all:
	$(MAKE) -C mxml -f Makefile
	$(MAKE) -C evalresp_log -f Makefile
	$(MAKE) -C spline -f Makefile
	$(MAKE) -C evalresp -f Makefile
	
clean:
	$(MAKE) -C mxml -f Makefile clean
	$(MAKE) -C evalresp_log -f Makefile clean
	$(MAKE) -C spline -f Makefile clean
	$(MAKE) -C evalresp -f Makefile clean
	
install:
	$(MAKE) -C mxml -f Makefile install
	$(MAKE) -C evalresp_log -f Makefile install
	$(MAKE) -C spline -f Makefile install
	$(MAKE) -C evalresp -f Makefile install
