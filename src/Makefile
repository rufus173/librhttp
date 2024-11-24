CFLAGS=-Wall -Wextra -g
CC=gcc
rufus-requests : tcp.o http.o main.o
	$(CC) $^ -o $@
librufushttp.so : tcp.o http.o
	$(CC) $^ -shared -o $@
tcp.o : tcp.c
	$(CC) $(CFLAGS) -fpic -c tcp.c
http.o : http.c tcp.h
	$(CC) $(CFLAGS) -fpic -c http.c
main.o : main.c http.h tcp.h
	$(CC) $(CFLAGS) -c main.c
