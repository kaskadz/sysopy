CC=gcc
OPTS=-Wall -Wextra

.PHONY: clean show_q clean_q test

all: server client

server: server.o protocol.h rainbow.h
	$(CC) $(OPTS) $^ -o $@ -lrt

client: client.o protocol.h rainbow.h
	$(CC) $(OPTS) $^ -o $@ -lrt

server.o: server.c protocol.h rainbow.h
	$(CC) -c $^ $(OPTS)

client.o: client.c
	$(CC) -c $^ $(OPTS)

test: server client
	./client in.txt
	./client < in.txt

show_q:
	@./show_queues.sh

clean_q:
	@./rm_queues.sh

clean:
	rm -f *.o client server
