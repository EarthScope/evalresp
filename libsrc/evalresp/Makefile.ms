
OBJ = alloc_fctns.obj calc_fctns.obj file_ops.obj \
			  regexp.obj regsub.obj resp_fctns.obj spline.obj input.obj\
			  output.obj stationxml2resp\wrappers.obj\
              highlevel.obj evaluation.obj legacy_interface.obj\
			  stationxml2resp\dom_to_seed.obj stationxml2resp\xml_to_dom.obj

all: evalresp.lib

.c.obj:
	$(CC) /Zi /c $*.c /I ..\ /I ..\mxml\ /Wall /DWIN32 /Fo$@

evalresp.lib: $(OBJ)
    link /lib $(OBJ) /debug /OUT:evalresp.lib

clean:
    del evalresp.lib
    del $(OBJ)

install: ..\..\install\lib
	copy evalresp.lib ..\..\install\lib\

..\..\install\lib:
	md ..\..\install\lib\
