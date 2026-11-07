CC = cc
CFLAGS = -Wall -Wextra -Werror -std=c11 -pedantic -g 
CPPFLAGS = -Iinclude

.PHONY: all clean

all: build/chip8

build/chip8: src/* include/*
	mkdir -p build
	$(CC) $(CFLAGS) $(CPPFLAGS) -o build/chip8 src/main.c

clean:
	rm -rf build
