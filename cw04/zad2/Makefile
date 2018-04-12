CC=gcc
OPTS=-Wall -Wextra
PROGNAME=program

.PHONY: clean test

all: $(PROGNAME) child

$(PROGNAME): main.o
	$(CC) $^ -o $@ $(OPTS)

main.o: main.c rainbow.h
	$(CC) -c $^ $(OPTS)

child: child.o
	$(CC) $^ -o $@ $(OPTS)

child.o: child.c rainbow.h
	$(CC) -c $^ $(OPTS)

clean:
	rm -f *.o $(PROGNAME)
