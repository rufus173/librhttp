#ifndef _HTTP_H
#define _HTTP_H
#include <arpa/inet.h>
#include <netdb.h>
struct http_connection {
	int socket;
	char *version_string;
	struct addrinfo *address_info;
};
struct header {
        char *field_name;
        char *field_value;
        void *next;
};
struct http_request {
	char *method;
	char *url;
	char *body;
	struct header *header;
};
struct http_response {
};
typedef struct http_response HTTP_response;
typedef struct http_request HTTP_request;
typedef struct http_connection HTTP_connection;
struct http_connection *http_connect(char *host);
int http_disconnect(HTTP_connection *);
int http_send_request(HTTP_connection *connection,HTTP_request *request);
HTTP_request *http_generate_request(char *method,char *url);
int http_free_request(HTTP_request *request);
HTTP_response *http_receive_response(HTTP_connection *);
int http_request_append_header(HTTP_request *, char *field_name, char *field_value);
#endif
