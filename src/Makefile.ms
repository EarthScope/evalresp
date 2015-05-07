
CM_OBJS = x2r_log.obj x2r_ws.obj x2r_xml.obj

EV_OBJS = alloc_fctns.obj calc_fctns.obj error_fctns.obj \
	evresp.obj evr_spline.obj file_ops.obj parse_fctns.obj \
	print_fctns.obj regerror.obj regexp.obj regsub.obj \
	resp_fctns.obj string_fctns.obj $(CM_OBJS) evalresp.obj

XR_OBJS = $(CM_OBJS) xml2resp.obj

all: evalresp.exe xml2resp.exe

evalresp.exe: $(EV_OBJS)
	link $(EV_OBJS) /out:evalresp.exe

xml2resp.exe: $(XR_OBJS)
	link $(XR_OBJS) /out:xml2resp.exe

.c.obj:
	cl /c /I ..\..\libxml2\include /I ..\..\iconv\include /I ..\..\zlib\include $<
