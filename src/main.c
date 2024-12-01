#include <stdio.h>
#include "http.h"
int main(int argc, char **argv){
	HTTP_connection *connection;
	connection = http_connect("google.com");
	if (connection == NULL){
		fprintf(stderr,"could not connect.\n");
		return 1;
	}
	HTTP_request *request;
	request = http_generate_request("GET","/");
	if (request == NULL){
		fprintf(stderr,"could not generate request\n");
		return 1;
	}
	http_send_request(connection,request);
	http_free_request(request);
	HTTP_response *response = http_receive_response(connection);
	if (response == NULL){
		fprintf(stderr,"failed to get response\n");
	}else{
		printf("%d: %s\n",response->status_code,response->status_message);
		if (response->body == NULL){
			printf("(no body)\n");
		}else{
			printf("%s\n",response->body);
		}
		http_free_response(response);
	}
	http_disconnect(connection);
	return 0;
}
