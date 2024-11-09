//================ headers =============
//#include "tcp.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
//=================== structs ================
struct http_connection {
	int socket;
	char *version_string;
};
struct http_connection *http_connect(char *host){
	struct http_connection *connection;
	connection = malloc(sizeof(http_connection));
	int sock = socket(AF_INET,SOCK_STREAM,0);
	if (sock < 0){
		perror("socket");
		return NULL;
	}
	struct addrinfo info, *result;
	int result = getaddrinfo(host,"80",&info,&result);
	if (result < 0){
		return NULL:
	}
	connection->socket = sock;
	return connection;
}
//================== public funcitons
int http_disconnect(struct http_connection *connection){
	close(connection->socket)
	free(struct http_connection);
	return 0;
}
