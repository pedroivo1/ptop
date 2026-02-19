# --- Configuration ---
CC = gcc
TARGET = ptop
TEST_TARGET = run_tests

# --- Flags ---
CFLAGS = -Wall -Wextra -Isrc -std=c2x -D_DEFAULT_SOURCE
RELEASE_FLAGS = -O3 -march=native -fno-stack-protector -flto -s
DEBUG_FLAGS = -O0 -g -fsanitize=address
TEST_FLAGS = $(DEBUG_FLAGS) -Isrc -Itest/unity -DUNITY_OUTPUT_COLOR

# --- Files ---
SRCS = $(shell find src -name "*.c")
# SRCS = src/main.c src/mod/cpu.c src/tui.c
TEST_SRCS = test/test_util.c test/unity/unity.c
COLORS_SRC = test/show_colors.c

.PHONY: all release debug clean test colors

all: debug

# --- ptop Compilation ---
release:
	$(CC) $(CFLAGS) $(RELEASE_FLAGS) $(SRCS) -o $(TARGET)

debug:
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) $(SRCS) -o $(TARGET)

# --- Tests ---
test:
	$(CC) $(CFLAGS) $(TEST_FLAGS) $(TEST_SRCS) -o $(TEST_TARGET)
	./$(TEST_TARGET)

# --- Tools ---
colors:
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) $(COLORS_SRC) -o show_colors
	./show_colors

clean:
	rm -f $(TARGET)
