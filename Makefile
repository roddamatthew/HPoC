CC = clang
CFLAGS = -Wall -Wextra -fPIC -I include

.PHONY: all coal free-list bump clean

bump: $(TEST_EXEC)
	$(CC) $(CFLAGS) include/utils.c 01-bump/bump.c 01-bump/main.c -o 01-bump/main

free-list: $(TEST_EXEC)
	$(CC) $(CFLAGS) include/utils.c 02-free-list/free-list.c 02-free-list/main.c -o 02-free-list/main

coal: $(TEST_EXEC)
	$(CC) $(CFLAGS) include/utils.c 03-coal/coal.c 03-coal/main.c -o 03-coal/main

clean:
	rm -f */main