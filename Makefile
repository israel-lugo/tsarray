
OBJ_FILES = dynarray.o test.o test2.o stacks.o
BIN_FILES = test test2


CFLAGS ?= -g -Wall

override CFLAGS += -I../include


all: tests


tests: test test2


test2: test2.o dynarray.o


test: test.o dynarray.o


clean:
	rm -f $(OBJ_FILES) $(BIN_FILES)


dynarray.o: dynarray.h common.h compiler.h
stacks.o: stacks.h dynarray.h common.h compiler.h
test2.o: dynarray.h common.h compiler.h
test.o: dynarray.h common.h compiler.h
