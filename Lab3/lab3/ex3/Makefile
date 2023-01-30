CC=gcc
CFLAGS=-g -Og -Wall -Wextra -std=gnu11 -Wno-unused-result
LDFLAGS=-pthread

.PHONY: clean

all: ex3
ex3: ex3.o packer.o
clean:
	rm -f ex3.o packer.o ex3
