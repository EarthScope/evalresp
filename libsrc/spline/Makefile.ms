
OBJ = spline.obj

all: spline.lib

.c.obj:
	$(CC) /c /I ..\ $<

spline.lib: $(OBJ)
	link /lib $(OBJ) /OUT:spline.lib

clean:
	del spline.lib
	del $(OBJ)
