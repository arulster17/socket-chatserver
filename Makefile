CC = gcc
CFLAGS = -Wall -Wextra -O2 

all: server client

server: server.c
	$(CC) $(CFLAGS) -o myserver server.c

client: client.c
	$(CC) $(CFLAGS) -o myclient client.c

clean:
	rm -f myserver myclient
