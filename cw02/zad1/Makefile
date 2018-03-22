CC=gcc
OPTS=-Wall -Wextra
PROGNAME=program

.PHONY: clean

all: $(PROGNAME)

$(PROGNAME): main.o libiocomp.a libmeasure.a
	$(CC) $^ -o $@ $(OPTS)

main.o: main.c
	$(CC) -c $^

libiocomp.a: iocomp.o
	ar cr $@ $^

iocomp.o: iocomp.c
	$(CC) -c $^

libmeasure.a: measure.o
	ar cr $@ $^

measure.o: measure.c
	$(CC) -c $^

test:
	./$(PROGNAME)

clean:
	rm -f *.o *.a program