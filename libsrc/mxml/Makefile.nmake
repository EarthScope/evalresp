
OBJ= mxml-attr.obj mxml-entity.obj mxml-file.obj mxml-get.obj \
     mxml-index.obj mxml-node.obj mxml-search.obj mxml-set.obj \
     mxml-private.obj mxml-string.obj

all: mxmlev.lib

.c.obj:
	$(CC) /Zi /c $*.c /I ..\ /Wall /DWIN32 /Fo$@

mxmlev.lib: config.h $(OBJ)
	link /lib $(OBJ) /debug /OUT:mxmlev.lib

config.h: config.h.ms
	copy config.h.ms config.h

clean:
	del mxmlev.lib
	del $(OBJ)

install: ..\..\install\lib
	copy mxmlev.lib ..\..\install\lib\

..\..\install\lib:
	md ..\..\install\lib\
