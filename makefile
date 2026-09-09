CC = gcc
STD = c2x
CFLAGS_COMMON  = -Wall -Wextra -pedantic-errors -std=$(STD)
CFLAGS_DEBUG   = -g -O0
CFLAGS_RELEASE = -O3
SRC = main.c scanner.c
BIN = calc
TEST_INPUT = entrada_prueba.txt

.PHONY: all debug release format test clean

all: debug

debug:
	$(CC) $(CFLAGS_COMMON) $(CFLAGS_DEBUG) -o $(BIN) $(SRC)

release:
	$(CC) $(CFLAGS_COMMON) $(CFLAGS_RELEASE) -o $(BIN) $(SRC)
	strip $(BIN)

format:
	clang-format -style=Google -i *.c *.h

test: debug
	./$(BIN) < $(TEST_INPUT)

clean:
	rm -f $(BIN)
