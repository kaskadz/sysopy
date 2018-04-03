CC=gcc
OPTS=-Wall -Wextra
PROGNAME=program

.PHONY: clean

all: $(PROGNAME)

$(PROGNAME): main.o
	$(CC) $^ -o $@ $(OPTS)

main.o: main.c
	$(CC) -c $^

test:
	./$(PROGNAME)

clean:
	rm -f *.o program
