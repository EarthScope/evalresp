
OBJ= mxml-attr.obj mxml-entity.obj mxml-file.obj mxml-get.obj \
     mxml-index.obi mxml-node.obj mxml-search.obj mxml-set.obj \
     mxml-private.obj mxml-string.obj

all: mxmlev.lib

.c.obj:
	$(CC) /c $*.c /I ..\ /Fo$@

mxmlev.lib: $(OBJ)
	link /lib $(OBJ) /OUT:mxmlev.lib

clean:
	del mxmlev.lib
	del $(OBJ)


