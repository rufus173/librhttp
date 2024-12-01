#ifndef _TCP_H
#define _TCP_H
#include <stddef.h>
int connect_socket(const char *address, const char *port);
int close_socket(int socket);
int tcp_recvall(int socket,char *buffer,size_t max);
#endif
