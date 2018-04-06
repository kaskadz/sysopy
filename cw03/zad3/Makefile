CC=gcc
OPTS=-Wall -Wextra
PROGNAME=program

.PHONY: clean

all: $(PROGNAME) cpu_hooligan mem_hooligan

$(PROGNAME): main.o rainbow.h
	$(CC) $^ -o $@ $(OPTS)

main.o: main.c
	$(CC) -c $^ $(OPTS)

clean:
	rm -f *.o $(PROGNAME) cpu_hooligan mem_hooligan

cpu_hooligan: cpu_hooligan.c
	$(CC) -o $@ $^ -lm $(OPTS)

mem_hooligan: mem_hooligan.c
	$(CC) -o $@ $^ $(OPTS)
