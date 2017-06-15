
CM_OBJS = x2r_log.obj x2r_ws.obj x2r_xml.obj

EV_OBJS = evalresp.obj

XR_OBJS = xml2resp.obj

LIBS = /libpath:..\libsrc\evalresp_log evalresp_log.lib\
       /libpath:..\libsrc\spline spline.lib\
       /libpath:..\libsrc\evalresp evalresp.lib\
       /libpath:..\libsrc\mxml mxmlev.lib

# don't forget that you also need (at the very least) before running
# compiled commands
# set PATH=%PATH%;..\..\libxml2\bin;..\..\iconv\bin;..\..\zlib\bin

all: evalresp.exe xml2resp.exe

evalresp.exe: $(EV_OBJS)
	link $(LIBS) $(EV_OBJS) /out:evalresp.exe

xml2resp.exe: $(XR_OBJS)
	link $(LIBS) $(XR_OBJS) /out:xml2resp.exe

.c.obj:
	cl /c /I ..\libsrc\mxml\ /I ..\libsrc\ /Wall $<
