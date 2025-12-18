CC = gcc
CFLAGS = -Wall -Wextra -fPIC -I include

.PHONY: all coal free-list bump clean

bump: $(TEST_EXEC)
	$(CC) $(CFLAGS) include/utils.c bump/bump.c bump/main.c -o bump/main

free-list: $(TEST_EXEC)
	$(CC) $(CFLAGS) include/utils.c free-list/free-list.c free-list/main.c -o free-list/main

coal: $(TEST_EXEC)
	$(CC) $(CFLAGS) include/utils.c coal/coal.c coal/main.c -o coal/main

clean:
	rm -f */main