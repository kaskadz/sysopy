CC=gcc
OPTS=-Wall -Wextra
SERVER=server
CLIENT=client

.PHONY: clean

all: $(SERVER) $(CLIENT)

$(PROGNAME): server.o protocol.h rainbow.h
	$(CC) $^ -o $@ $(OPTS)

$(CLIENT): client.o
	$(CC) $^ -o $@ $(OPTS)

server.o: server.c protocol.h rainbow.h
	$(CC) -c $^ $(OPTS)

client.o: client.c
	$(CC) -c $^ $(OPTS)

clean:
	rm -f *.o $(SERVER) $(CLIENT)
