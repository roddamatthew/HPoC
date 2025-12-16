CC = gcc
CFLAGS = -Wall -Wextra -fPIC -I include
LDFLAGS = -L. -lhpoc -Wl,-rpath=$(shell pwd)

# Target names
LIB_NAME = libhpoc.so
TEST_EXEC = test.o

.PHONY: all clean library test

all: $(TEST_EXEC)

library: $(LIB_NAME)

$(LIB_NAME): src/hpoc.c include/hpoc.h
	$(CC) $(CFLAGS) -shared -o $(LIB_NAME) src/hpoc.c

$(TEST_EXEC): tests/test.c $(LIB_NAME)
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

test: $(TEST_EXEC)
	export LD_LIBRARY_PATH=$(shell pwd):$(LD_LIBRARY_PATH); ./$(TEST_EXEC)

clean:
	rm -f $(LIB_NAME) $(TEST_EXEC) *.o