
OBJ_FILES = tsarray.o test.o test2.o
BIN_FILES = test test2


CFLAGS ?= -g -Wall


all: tests


tests: test test2


test2: test2.o tsarray.o


test: test.o tsarray.o


clean:
	rm -f $(OBJ_FILES) $(BIN_FILES)


tsarray.o: tsarray.h common.h compiler.h
test2.o: tsarray.h common.h compiler.h
test.o: tsarray.h common.h compiler.h
