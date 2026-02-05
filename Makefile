CC = gcc
CFLAGS = -Wall -I.
OPCFLAGS = -O3 -march=native -fno-stack-protector -flto -s
SRCS = src/main.c src/modules/cpu.c src/tui.c

.PHONY: all ptop

all:
	$(CC) $(CFLAGS) $(OPCFLAGS) $(SRCS) -o ptop
