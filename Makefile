CC = gcc
CFLAGS = -Wall -I.
OPCFLAGS = -O3 -march=native -fno-stack-protector -flto
HEADERS = src/utils.h src/config.h

.PHONY: all run test clean cpumon

# Compilation
all: src/main.c src/utils.c $(HEADERS)
	$(CC) $(CFLAGS) src/main.c src/utils.c -o ptop

cpumon:
	$(CC) $(CFLAGS) $(OPCFLAGS) src/cpumon.c -o cpumon

# Tests
t: src/test.c src/utils.c $(HEADERS)
	$(CC) $(CFLAGS) -g src/test.c src/utils.c -o test

vtest: t
	valgrind --leak-check=full --track-origins=yes -s ./test > /dev/null

test: t
	./test

# Clean
clean:
	rm -f ptop test cpumon
