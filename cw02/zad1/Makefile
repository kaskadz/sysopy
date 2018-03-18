CC=gcc
OPTS=-Wall -Wextra
PROGNAME=program

.PHONY: clean

all: $(PROGNAME)

$(PROGNAME): main.c libmeasure.a
	$(CC) $^ -o $@ $(OPTS)

test:
	./$(PROGNAME)

libmeasure.a: measure.o
	ar cr $@ $^

measure.o: measure.c
	$(CC) -c $^

clean:
	rm -f *.o *.a program program.exe