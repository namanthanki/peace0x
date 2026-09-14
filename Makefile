CC = gcc
CFLAGS ?= -std=c17 -O3 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wno-sign-conversion -Iinclude
DEBUG_FLAGS ?= -std=c17 -g -DDEBUG -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wno-sign-conversion -Iinclude

ifeq ($(OS),Windows_NT)
    EXE = peace0x.exe
    TEST_EXE = perft_test.exe
    RM = del /Q /F
else
    EXE = peace0x
    TEST_EXE = perft_test
    RM = rm -f
endif

SRC = $(wildcard src/*.c)
CORE_SRC = $(filter-out src/main.c, $(SRC))

all: $(EXE)

$(EXE): $(SRC)
	$(CC) $(CFLAGS) $^ -o $@

debug: $(SRC)
	$(CC) $(DEBUG_FLAGS) $^ -o $(EXE)

test: $(TEST_EXE)
	./$(TEST_EXE)

$(TEST_EXE): tests/perft_suite.c $(CORE_SRC)
	$(CC) $(CFLAGS) $^ -o $@

clean:
	$(RM) $(EXE) $(TEST_EXE) *.o .deps 2>nul || true

.PHONY: all debug test clean