#ifndef _HTTP_H
#define _HTTP_H
#include <arpa/inet.h>
#include <netdb.h>
struct http_connection {
	int socket;
	char *version_string;
	struct addrinfo *address_info;
};
struct http_request {
	char *method;
	char *url;
};
typedef struct http_request HTTP_request;
typedef struct http_connection HTTP_connection;
struct http_connection *http_connect(char *host);
int http_disconnect(HTTP_connection *);
int http_send_request(HTTP_connection *connection,HTTP_request *request);
HTTP_request *http_generate_request(char *method,char *url);
int http_free_request(HTTP_request *request);
#endif
