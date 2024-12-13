#ifndef _HTTP_H
#define _HTTP_H
#include <arpa/inet.h>
#include <netdb.h>
struct http_connection {
	int socket;
	char *host;
	char *port;
	char *version_string;
	struct addrinfo *address_info;
	int use_ssl;
};
struct http_header {
        char *field_name;
        char *field_value;
        void *next;
};
struct http_request {
	char *method;
	char *url;
	char *body;
	size_t body_size;
	struct http_header *header;
};
struct http_response {
	int status_code;
	char *status_message;
	struct http_header *header;
	char *body;
	size_t body_size;
};
typedef struct http_response HTTP_response;
typedef struct http_request HTTP_request;
typedef struct http_connection HTTP_connection;
struct http_connection *http_connect(char *host, char *port);
int http_disconnect(HTTP_connection *);
int http_send_request(HTTP_connection *connection,HTTP_request *request);
HTTP_request *http_generate_request(char *method,char *url);
int http_free_request(HTTP_request *request);
int http_free_response(struct http_response *);
HTTP_response *http_receive_response(HTTP_connection *);
int http_request_append_header(HTTP_request *, char *field_name, char *field_value);
int http_request_add_body(HTTP_request*, char *body, size_t body_size);
#endif
