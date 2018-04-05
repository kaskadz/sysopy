CC=gcc
OPTS=-Wall -Wextra
PROGNAME=program

.PHONY: clean

all: $(PROGNAME)

$(PROGNAME): main.o rainbow.h
	$(CC) $^ -o $@ $(OPTS)

main.o: main.c
	$(CC) -c $^

test:
	./$(PROGNAME)

clean:
	rm -f *.o program

cpu_hooligan: cpu_hooligan.c
	$(CC) -o $@ $^

mem_hooligan: mem_hooligan.c
	$(CC) -o $@ $^
