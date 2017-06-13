
OBJ = alloc_fctns.obj calc_fctns.obj file_ops.obj libevalresp.obj parse_fctns.obj\
			  regexp.obj regsub.obj resp_fctns.obj spline.obj string_fctns.obj\
			  print_fctns.obj error_fctns.obj\
			  stationxml2resp/log.obj stationxml2resp/ws.obj stationxml2resp/xml.obj

all: evalresp.lib

.c.obj:
    $(CC) /c /I ..\ /I ..\mxml\ $<

evalresp.lib: $(OBJ)
    $(LINK) /lib $< /OUT:evalresp.lib

clean:
    del evalresp.lib
    del $(OBJ)

