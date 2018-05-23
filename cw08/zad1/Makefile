CC=gcc
OPTS=-Wall -Wextra

.PHONY: clean test

all: main

main: main.o rainbow.h
	$(CC) $(OPTS) $^ -o $@ -lpthread

main.o: main.c
	$(CC) -c $^ $(OPTS)

test: main
	./main

clean:
	rm -f *.o main
