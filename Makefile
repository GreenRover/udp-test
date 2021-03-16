all: client server

client: client.c common.h
	$(CC) -O3 $< -o $@

server: server.c common.h
	$(CC) -O3 $< -o $@

