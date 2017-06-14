
OBJ=log.obj examples/to_file.obj

all: evalresp_log.lib

.c.obj:
	$(CC) /c $*.c /I ..\ /Fo$@

evalresp_log.lib: $(OBJ)
	link /lib $(OBJ) /OUT:evalresp_log.lib

clean:
	del evalresp_log.lib
	del $(OBJ)

