#ifndef _TCP_H
#define _TCP_H
#include <stddef.h>
struct http_connection;
int tcp_connect_socket(struct http_connection *,char *address,char *port);
int tcp_close_socket(struct http_connection *);
int tcp_recvall(struct http_connection *,char *buffer,size_t max);
int tcp_recv_to_crlf(struct http_connection *, char **buffer);
int tcp_sendall(struct http_connection *, void *buffer, size_t length, int flags);
#endif
