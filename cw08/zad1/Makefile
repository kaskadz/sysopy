CC=gcc
OPTS=-Wall -Wextra

.PHONY: clean test

all: main

main: main.o rainbow.h
	$(CC) $(OPTS) $^ -o $@ -lpthread -lm # -g -fsanitize=address -fsanitize=undefined -fsanitize=leak -lasan -lubsan

main.o: main.c
	$(CC) -c $^ $(OPTS)

test: main
	./main

clean:
	rm -f *.o main
