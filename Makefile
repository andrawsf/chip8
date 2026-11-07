CC = cc
CFLAGS = -Wall -Wextra -Werror -std=c11 -pedantic -g 
CPPFLAGS = -Iinclude

.PHONY: all test clean

all: build/chip8

build/chip8: src/* include/*
	mkdir -p build
	$(CC) $(CFLAGS) $(CPPFLAGS) -o build/chip8 src/main.c src/chip8.c

test: tests/* include/*
	mkdir -p build
	$(CC) $(CFLAGS) $(CPPFLAGS) -o build/tests_chip8 tests/test_chip8.c src/chip8.c
	./build/tests_chip8

clean:
	rm -rf build
