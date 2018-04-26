CC=gcc
OPTS=-Wall -Wextra
SERVER=server
CLIENT=client

.PHONY: clean show_q clean_q

all: $(SERVER) $(CLIENT)

$(PROGNAME): server.o protocol.h rainbow.h
	$(CC) $^ -o $@ $(OPTS)

$(CLIENT): client.o
	$(CC) $^ -o $@ $(OPTS)

server.o: server.c protocol.h rainbow.h
	$(CC) -c $^ $(OPTS)

client.o: client.c
	$(CC) -c $^ $(OPTS)

show_q:
	@./show_queues.sh

clean_q:
	@./rm_queues.sh

clean:
	rm -f *.o $(SERVER) $(CLIENT)
