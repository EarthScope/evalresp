
OBJ= mxml-attr.obj mxml-entity.obj mxml-file.obj mxml-get.obj \
     mxml-index.obi mxml-node.obj mxml-search.obj mxml-set.obj \
     mxml-private.obj mxml-string.obj

all: mxmlev.lib

.c.obj: config.h
	$(CC) /c $*.c /I ..\ /Fo$@

mxmlev.lib: $(OBJ)
	link /lib $(OBJ) /OUT:mxmlev.lib

config.h: config.h.in
	copy config.h.in config.h

clean:
	del mxmlev.lib
	del $(OBJ)


