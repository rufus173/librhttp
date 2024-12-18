CFLAGS=-Wall -Wextra -g
LFLAGS=-lssl -lcrypto
CC=gcc
http-request : tcp.o http.o main.o command_interface.o
	$(CC) $^ $(LFLAGS) -o $@
librufushttp.so : tcp.o http.o
	$(CC) $^ -shared -o $@
tcp.o : tcp.c http.h
	$(CC) $(CFLAGS) -fpic -c tcp.c
http.o : http.c tcp.h
	$(CC) $(CFLAGS) -fpic -c http.c
main.o : main.c http.h tcp.h
	$(CC) $(CFLAGS) -c main.c
command_interface.o : command_interface.h command_interface.c
	$(CC) $(CFLAGS) -c command_interface.c
