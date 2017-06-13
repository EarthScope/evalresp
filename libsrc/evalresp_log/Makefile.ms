
OBJ=log.obj examples/to_file.obj

all: evalresp_log.lib

.c.obj:
    $(CC) /c /I ..\ $<

evalresp_log.lib: $(OBJ)
    $(LINK) /lib $< /OUT:evalresp_log.lib

clean:
    del evalresp_log.lib
    del $(OBJ)

