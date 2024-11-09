CFLAGS=-Wall -g
CC=gcc
rufus-requests : tcp.o http.o main.o
	$(CC) main.o tcp.o http.o -o rufus-requests
libhttpr.so : tcp.o http.o
	$(CC) tcp.o http.o -shared -o librhttp.so
tcp.o : tcp.c
	$(CC) $(CFLAGS) -fpic -c tcp.c
http.o : http.c tcp.h
	$(CC) $(CFLAGS) -fpic -c http.c
main.o : main.c http.h tcp.h
	$(CC) $(CFLAGS) -c main.c
