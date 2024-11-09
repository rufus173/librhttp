//================ headers =============
//#include "tcp.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
//=================== structs ================
struct http_connection {
	int socket;
	char *version_string;
	struct addrinfo *address_info;
};
struct http_request {
        char *method;
        char *url;
};
//================== public funcitons =============
struct http_connection *http_connect(char *host){
	//setup returned struct
	struct http_connection *connection;
	connection = malloc(sizeof(struct http_connection));

	//resolve a hostname or url
	struct addrinfo hints, *address_info;
	memset(&hints,0,sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	int result = getaddrinfo(host,"80",&hints,&address_info);
	if (result < 0){
		fprintf(stderr,"getaddrinfo: %s\n",gai_strerror(result));
		return NULL;
	}
	connection->address_info = address_info;

	//create a socket
	int sock = socket(AF_INET,SOCK_STREAM,0);
	if (sock < 0){
		perror("socket");
		return NULL;
	}
	connection->socket = sock;

	//connect
	result = connect(sock,address_info->ai_addr,address_info->ai_addrlen);
	if (result < 0){
		perror("connect");
		return NULL;
	}

	return connection;
}
int http_disconnect(struct http_connection *connection){
	close(connection->socket);
	freeaddrinfo(connection->address_info);
	free(connection);
	return 0;
}
int http_send_request(struct http_connection *connection,struct http_request *request){
	int request_line_size = snprintf(NULL,0,"%s %s HTTP/1.1",request->method,request->url)+1;//get size of request line
	char *request_line = malloc(request_line_size);
	snprintf(request_line,request_line_size,"%s %s HTTP/1.1",request->method,request->url);
	

	free(request_line);
	return 0;
}
struct http_request *http_generate_request(char *method, char *url){
	struct http_request *request;
	request = malloc(sizeof(struct http_request));
	if (request == NULL){
		return NULL;
	}
	request->method = malloc(strlen(method)+1);
	request->url = malloc(strlen(url)+1);
	if (request->method == NULL || request->url == NULL){
		perror("malloc");
		return NULL;
	}
	snprintf(request->method,strlen(method)+1,"%s",method);
	snprintf(request->url,strlen(url)+1,"%s",url);
	return request;
}
int http_free_request(struct http_request *request){
	free(request->method);
	free(request->url);
	free(request);
	return 0;
}
