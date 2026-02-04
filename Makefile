CC = gcc
CFLAGS = -Wall -I.
OPCFLAGS = -O3 -march=native -fno-stack-protector -flto -s
SRCS = src/main.c src/modules/cpu.c src/tui.c

.PHONY: all run test clean cpumon

cpumon:
	$(CC) $(CFLAGS) $(OPCFLAGS) $(SRCS) -o cpumon
