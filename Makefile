
OBJ_FILES = tsarray.o tssparse.o test-sparse.o test2-sparse.o
TEST_BIN_FILES = test-array test-sparse test2-sparse
BIN_FILES = $(TEST_BIN_FILES)


CFLAGS ?= -g -Wall


all: tests tsarray.o


tests: $(TEST_BIN_FILES)


test-array: test-array.o tsarray.o


test2-sparse: test2-sparse.o tssparse.o


test-sparse: test-sparse.o tssparse.o


clean:
	rm -f $(OBJ_FILES) $(BIN_FILES)

# DO NOT DELETE

tsarray.o: tsarray.h common.h compiler.h
tssparse.o: tssparse.h common.h compiler.h
test2-sparse.o: tssparse.h common.h compiler.h
test-array.o: tsarray.h common.h compiler.h
test-sparse.o: tssparse.h common.h compiler.h
