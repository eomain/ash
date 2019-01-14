
# ash (acorn shell) - 2018

OUT = ash

CC = cc

CFLAGS := -I ../include

BIN = ../bin

OBJS = ash.o io.o env.o var.o builtin.o \
	   lang.o mem.o script.o exec.o echo.o \
	   cd.o sleep.o unset.o ops.o

all: $(OBJS)
	-@mkdir -p $(BIN)
	$(CC) $(CFLAGS) $(OBJS) -o $(BIN)/$(OUT)

%.o:%.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	-@rm -r $(BIN) $(OBJS)
