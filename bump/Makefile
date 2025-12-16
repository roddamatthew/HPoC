CC = gcc
CFLAGS = -Wall -Wextra -fPIC -I ../shared
LDFLAGS = -L. -lhpoc -Wl,-rpath=$(shell pwd)

# Target names
LIB_NAME = libbump.so
TEST_EXEC = main

.PHONY: all clean library test

all: $(TEST_EXEC)

library: $(LIB_NAME)

$(LIB_NAME): bump.c ../shared/chunk.h
	$(CC) $(CFLAGS) -shared -o $(LIB_NAME) bump.c

$(TEST_EXEC): main.c $(LIB_NAME)
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

test: $(TEST_EXEC)
	export LD_LIBRARY_PATH=$(shell pwd):$(LD_LIBRARY_PATH); ./$(TEST_EXEC)

clean:
	rm -f $(LIB_NAME) $(TEST_EXEC) *.o