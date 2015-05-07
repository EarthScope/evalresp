
OBJS = alloc_fctns.obj calc_fctns.obj error_fctns.obj evalresp.obj evresp.obj \
	evr_spline.obj file_ops.obj parse_fctns.obj print_fctns.obj \
	regerror.obj regexp.obj regsub.obj resp_fctns.obj string_fctns.obj \
	x2r_log.obj x2r_ws.obj x2r_xml.obj xml2resp.obj

all: evalresp.exe

evalresp.exe: $(OBJS)
	$(link) $(conlflags) $(ldebug) $(conlibsmt) $(OBJS) -out:evalresp.exe

.c.obj:
	cl /c /I ..\..\libxml2\include /I ..\..\iconv\include /I ..\..\zlib\include $<
