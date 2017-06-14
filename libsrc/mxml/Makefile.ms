
OBJ= mxml-attr.o mxml-entity.o mxml-file.o mxml-get.o \
     mxml-index.o mxml-node.o mxml-search.o mxml-set.o \
     mxml-private.o mxml-string.o

all: mxmlev.lib

.c.obj:
	$(CC) /c /I ..\ $<

mxmlev.lib: $(OBJ)
	link /lib $(OBJ) /OUT:mxmlev.lib

clean:
	del mxmlev.lib
	del $(OBJ)


