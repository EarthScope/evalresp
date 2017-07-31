
OBJ= mxml-attr.obj mxml-entity.obj mxml-file.obj mxml-get.obj \
     mxml-index.obj mxml-node.obj mxml-search.obj mxml-set.obj \
     mxml-private.obj mxml-string.obj

all: mxmlev.lib

.c.obj:
	$(CC) /Zi /c $*.c /I ..\ /Wall /DWIN32 /Fo$@

mxmlev.lib: config.h $(OBJ)
	link /DEBUG /lib $(OBJ) /OUT:mxmlev.lib

config.h: config.h.in
	copy config.h.in config.h

clean:
	del mxmlev.lib
	del $(OBJ)


