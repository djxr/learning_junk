CC=cc
CFLAGS=-std=c99 -Wall -g -pedantic
LDFLAGS=-lSDL2 -lm

all: main

run:
	./main

clean:
	rm -f main
