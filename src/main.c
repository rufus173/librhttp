#include <stdio.h>
#include "http.h"
int main(int argc, char **argv){
	HTTP_connection *connection;
	connection = http_connect("www.google.com");
	if (connection == NULL){
		fprintf(stderr,"could not connect.\n");
		return 1;
	}
	HTTP_request *request;
	request = http_generate_request("GET","www.google.com");

	http_send_request(connection,request);

	http_free_request(request);
	http_receive_response(connection);

	http_disconnect(connection);
	return 0;
}
