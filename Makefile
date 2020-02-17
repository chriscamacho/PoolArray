
LDFLAGS:=-lm

CFLAGS:= -Werror -Wfatal-errors -pedantic -Wall -Wextra
CFLAGS+= -std=c99 -I ./include

SRC:=$(wildcard src/*.c)
OBJ:=$(SRC:src/%.c=obj/%.o)
INC:=$(wildcard include/*.h)

CC=gcc

poolArrayTest: $(OBJ)
	$(CC) $(OBJ) -o poolArrayTest $(LDFLAGS)

$(OBJ): obj/%.o : src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: debug release
debug: CFLAGS+= -g
release: CFLAGS+= -O3

debug release: clean poolArrayTest

.PHONY:	clean
clean:
	rm obj/* -f
	rm poolArrayTest -f

