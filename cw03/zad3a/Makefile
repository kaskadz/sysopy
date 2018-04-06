CC=gcc
OPTS=-Wall -Wextra
PROGNAME=program

.PHONY: clean

all: $(PROGNAME)

$(PROGNAME): main.o rainbow.h
	$(CC) $^ -o $@ $(OPTS)

main.o: main.c
	$(CC) -c $^ $(OPTS)

clean:
	rm -f *.o $(PROGNAME)
