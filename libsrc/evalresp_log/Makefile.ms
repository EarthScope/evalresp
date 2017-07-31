
OBJ=log.obj examples\to_file.obj

all: evalresp_log.lib

.c.obj:
	$(CC) /Zi /c $*.c /I ..\ /Wall /Fo$@

evalresp_log.lib: $(OBJ)
	link /lib $(OBJ) /debug /OUT:evalresp_log.lib

clean:
	del evalresp_log.lib
	del $(OBJ)

