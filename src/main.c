#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "http.h"
#include "command_interface.h"
int display_help();
int main(int argc, char **argv){
	int return_status = 0;
	//command line mode
	if (argc == 1){
		return command_line_mode();
	}

	//normal argument mode
	int processed_argc = 0;
	char **processed_argv = NULL;
	//================== process arguments ==================
	for (int i = 1; i < argc; i++){
		if (strcmp(argv[i],"--help") == 0) return display_help();
		//appened non "-args" or "--long-args" to the processed_argv
		if (argv[i][0] != '-'){
			processed_argc++;
			processed_argv = realloc(processed_argv,processed_argc*sizeof(char*));
			processed_argv[processed_argc-1] = argv[i]; //no need to alloc or dealloc
		}
	}
	if (processed_argc < 1){
		printf("Not enough arguments.\n");
		display_help();
		goto cleanup;
	}
	//--------------- url processing ---------------
	//get the protocol
	// protocol
	// |    port
	// |    |             
	//"http:80//google.com/google"
	//     ^  ^ |
	//     '\0' host
	char *protocol = NULL;
	char *port = NULL;
	char *host = NULL;
	char *path = NULL;
	protocol = processed_argv[0];
	host = strstr(protocol,"://");
	if (host == NULL){
		fprintf(stderr,"Bad URL.\n");
		return_status = -1;
		goto cleanup;
	}
	host += 3;
	host[-3] = '\0';
	port = strchr(host,':');
	if (port == NULL){
		port = "80"; //fallback if port is not set
	}
	path = strchr(host,'/');
	if (path == NULL) path = "/";
	printf("protocol:%s\nport:%s\nhost:%s\npath:%s\n",protocol,port,host,path);

	HTTP_connection *connection;
	connection = http_connect(host,port);
	if (connection == NULL){
		fprintf(stderr,"could not connect.\n");
		return 1;
	}
	HTTP_request *request;
	request = http_generate_request("GET",path);
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
	
	//clean up loose pointers
	cleanup:
	free(processed_argv);
	return return_status;
}
int display_help(){
	printf("Usage: http-request [options] <url>\n");
	printf("options:\n--GET, --HEAD, --POST, --DELETE, --TRACE, --OPTIONS, --PUT, --PATCH : Use said selected method.\n");
	return 0;
}
