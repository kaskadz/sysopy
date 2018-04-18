CC=gcc
OPTS=-Wall -Wextra
PROGNAME=program

.PHONY: clean test1 test2

all: $(PROGNAME)

$(PROGNAME): main.o
	$(CC) $^ -o $@ $(OPTS)

main.o: main.c
	$(CC) -c $^ $(OPTS)

test1:
	./$(PROGNAME) in1.txt

test2:
	./$(PROGNAME) in2.txt

clean:
	rm -f *.o program
