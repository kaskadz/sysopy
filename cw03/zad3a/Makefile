CC=gcc
OPTS=-Wall -Wextra
PROGNAME=program

.PHONY: clean test

all: $(PROGNAME)

$(PROGNAME): main.o rainbow.h
	$(CC) $^ -o $@ $(OPTS)

main.o: main.c
	$(CC) -c $^ $(OPTS)

test:
	./$(PROGNAME) in.txt

clean:
	rm -f *.o $(PROGNAME)
