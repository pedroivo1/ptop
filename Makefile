CC = gcc
CFLAGS = -Wall -I.

.PHONY: all run test clean

# Compilation
all:
	$(CC) $(CFLAGS) src/main.c src/utils.c -o ptop

# Tests
t:
	$(CC) $(CFLAGS) -g src/test.c src/utils.c -o test

vtest:
	valgrind --leak-check=full --track-origins=yes -s ./test > /dev/null

test:
	./test

# Clean
clean:
	rm -f ptop test
