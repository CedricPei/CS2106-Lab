CC=gcc
CFLAGS=-g -Og -Wall -Wextra -std=gnu11 -Wno-unused-result
LDFLAGS=-pthread

.PHONY: clean

all: ex2
ex2: ex2.o packer.o
clean:
	rm -f ex2.o packer.o ex2
