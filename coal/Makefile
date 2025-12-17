CC = gcc
CFLAGS = -Wall -Wextra -fPIC -I ../shared

.PHONY: all clean library test

all: $(TEST_EXEC)
	$(CC) $(CFLAGS) coal.c main.c -o main

clean:
	rm -f main