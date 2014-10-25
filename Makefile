# setting compiler and compile options
CC      = gcc
LD      = ld
CFLAGS  = -ggdb -MD -Wall -Werror -fno-strict-aliasing -I./include -O

# target to compile
CFILES  = $(shell find src/ -name "*.c")
OBJS    = $(CFILES:.c=.o)

scheme: $(OBJS)
	$(CC) -o scheme $(OBJS) $(CFLAGS) -lreadline

run: scheme
	./scheme -d 2>&1 | tee log.txt

interpret: scheme
	./scheme -s sample.scm 2>&1 | tee log.txt

gdb: scheme
	gdb --args ./scheme -dq 

-include $(OBJS:.o=.d)

clean:
	/bin/rm -f scheme $(OBJS) $(OBJS:.o=.d) log.txt 2> /dev/null
