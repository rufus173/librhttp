#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "http.h"
#include "command_interface.h"
#define FLAG_SILENT 1
int display_help();
int main(int argc, char **argv){
	int return_status = 0;
	uint64_t flags = 0;
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
		if (strcmp(argv[i],"--silent") == 0) flags |= FLAG_SILENT;
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
	if (path == NULL) {
		path = "/";
	}else{
		path[0] = '\0'; //we put back the '/' after we are done with the host
	}
	printf("protocol:%s\nport:%s\nhost:%s\n",protocol,port,host);

	HTTP_connection *connection;
	connection = http_connect(host,port);
	if (connection == NULL){
		fprintf(stderr,"could not connect.\n");
		return 1;
	}

	//put back the '/' which was replaced with '\0' to terminate the string without reallocating it
	if (path[0] == '\0') path[0] = '/';
	printf("path:%s\n",path);

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
		if ((flags & FLAG_SILENT) == 0){
			printf("%d: %s\n",response->status_code,response->status_message);
			if (response->body == NULL){
				printf("(no body)\n");
			}else{
				printf("%s\n",response->body);
			}
		}
		http_free_response(response);
	}
	
	//clean up loose pointers
	cleanup:
	http_disconnect(connection);
	free(processed_argv);
	return return_status;
}
int display_help(){
	printf("Usage: http-request [options] <url>\n");
	printf("options:\n--GET, --HEAD, --POST, --DELETE, --TRACE, --OPTIONS, --PUT, --PATCH : Use said selected method.\n");
	return 0;
}
