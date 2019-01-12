
# ash (acorn shell) - 2018

BIN = bin

OBJS = ash.o io.o env.o var.o builtin.o \
	   lang.o mem.o script.o exec.o echo.o \
	   cd.o sleep.o unset.o

ash: $(OBJS)
	-@mkdir -p $(BIN)
	$(CC) $(CFLAGS) $(OBJS) -o $(BIN)/$@

%.o:%.c
	$(CC) -c $(CFLAGS) -I include $< -o $@

clean:
	-@rm -r $(BIN) $(OBJS)
