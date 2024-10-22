CC = g++
CFLAGS = -Wall -g

all: server subscriber

server:
	$(CC) $(CFLAGS) server.cpp common.cpp -o server

subscriber:
	$(CC) $(CFLAGS) subscriber.cpp common.cpp -o subscriber

.PHONY: clean

clean:
	rm -f server subscriber