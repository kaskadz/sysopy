CC=gcc
OPTS=-Wall -Wextra
PROGNAME=program

.PHONY: clean

all: $(PROGNAME)

$(PROGNAME): main.c
	$(CC) -o $@ $^

test:
	./$(PROGNAME)
