
OBJ = alloc_fctns.obj calc_fctns.obj file_ops.obj libevalresp.obj parse_fctns.obj\
			  regexp.obj regsub.obj resp_fctns.obj spline.obj string_fctns.obj input.obj\
			  print_fctns.obj error_fctns.obj output.obj stationxml2resp\wrappers.obj\
              highlevel.obj evaluation.obj\
			  stationxml2resp\dom_to_seed.obj stationxml2resp\xml_to_dom.obj

all: evalresp.lib

.c.obj:
	$(CC) /c $*.c /I ..\ /I ..\mxml\ /Wall /Fo$@

evalresp.lib: $(OBJ)
    link /lib $(OBJ) /OUT:evalresp.lib

clean:
    del evalresp.lib
    del $(OBJ)

