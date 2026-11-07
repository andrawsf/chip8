CC = cc
CFLAGS = -Wall -Wextra -Werror -std=c11 -pedantic -g

.PHONY: all clean

all: build/chip8

build/chip8: src/* include/*
	mkdir -p build
	$(CC) $(CFLAGS) -o build/chip8 src/main.c

clean:
	rm -rf build
