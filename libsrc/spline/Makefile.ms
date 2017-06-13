
OBJ = splin.obj

all: spline.lib

.c.obj:
    $(CC) /c /I ..\ $<

spline.lib: $(OBJ)
    $(LINK) /lib $< /OUT:spline.lib

clean:
    del spline.lib
    del $(OBJ)

