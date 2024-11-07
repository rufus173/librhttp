CFLAGS=-Wall
CC=gcc
librhttp.so : tcp.o http.o
	$(cc) -shared -o librhttp.so
tcp.o : tcp.c
	$(CC) $(CFLAGS) -fpic -c tcp.c
tcp.o : http.c tcp.h
	$(CC) $(CFLAGS) -fpic -c http.c
