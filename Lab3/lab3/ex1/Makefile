CC=gcc
CFLAGS=-g -Og -Wall -Wextra -std=gnu11 -Wno-unused-result
LDFLAGS=-pthread

.PHONY: clean

all: ex1
ex1: ex1.o packer.o
clean:
	rm -f ex1.o packer.o ex1
