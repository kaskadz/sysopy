CC=gcc
OPTS=-Wall -Wextra
PROGNAME=program

.PHONY: clean

all: $(PROGNAME)

$(PROGNAME): main.c libmeasure.a libiocomp.a
	$(CC) $^ -o $@ $(OPTS)

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
	rm -f *.o *.a program program.exe