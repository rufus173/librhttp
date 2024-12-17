#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "http.h"
#include "command_interface.h"
#define FLAG_SILENT 1
#define FLAG_ONLY_BODY 2
#define FLAG_SSL 4
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
		if (strcmp(argv[i],"--body-only") == 0) flags |= FLAG_ONLY_BODY;
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
		if (strcmp(protocol,"http") == 0) port = "80";
		if (strcmp(protocol,"https") == 0){
			port = "443";
			flags |= FLAG_SSL;
		}
	}else{
		port[0] = '\0';
		port++;
	}
	path = strchr(host,'/');
	if (path == NULL) {
		path = "/";
	}else{
		path[0] = '\0'; //we put back the '/' after we are done with the host
	}
	printf("protocol:%s\nport:%s\nhost:%s\n",protocol,port,host);

	int connect_flags = 0;
	if ((flags & FLAG_SSL) > 0) connect_flags |= CONNECT_FLAG_USE_SSL;
	HTTP_connection *connection;
	connection = http_connect(host,port,connect_flags);
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
		//dont output if silent
		if ((flags & FLAG_SILENT) == 0){
			if ((flags & FLAG_ONLY_BODY) == 0){
				printf("====== status ======\n");
				printf("%d: %s\n",response->status_code,response->status_message);
				printf("====== headers ======\n");
				http_print_headers(response->header);
				printf("====== body ======\n");
			}
			if (response->body == NULL){
				printf("(no body)\n");
			}else{
				//printf("body size: %lu\n",response->body_size);
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
	printf("--silent : dont print response\n");
	printf("--body-only : only print the body without the formatting\n");
	return 0;
}
