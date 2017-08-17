
OBJ_FILES = tssparse.o test.o test2.o
BIN_FILES = test test2


CFLAGS ?= -g -Wall


all: tests


tests: test test2


test2: test2.o tssparse.o


test: test.o tssparse.o


clean:
	rm -f $(OBJ_FILES) $(BIN_FILES)


tssparse.o: tssparse.h common.h compiler.h
test2.o: tssparse.h common.h compiler.h
test.o: tssparse.h common.h compiler.h
