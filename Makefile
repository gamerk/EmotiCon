CC := gcc
CFLAGS := -Wall -Wextra -Wstrict-prototypes -pedantic -gdwarf-4 -Werror

OBJS := lex.o list.o

.PHONY: all clean

all: emoticon.exe tests.exe

emoticon.exe: emoticon.o $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

clean:
	- rm *.o
	- rm *.exe
	- rm *.stackdump

%.o: %.c
	$(CC) $(CFLAGS) -c $^

tests.exe: tests.o $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@