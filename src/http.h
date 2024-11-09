#ifndef _HTTP_H
#define _HTTP_H
struct http_connection {
	int socket;
	char *version_string;
};
struct http_connection *http_connect(char *host)
int http_disconnect(struct http_connection *);
#endif
