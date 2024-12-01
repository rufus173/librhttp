//================ headers =============
#include "tcp.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
//================ macros ==============
#define CHECK_SEND(result,return_val) \
if (result < 0){\
	perror("send");\
	return return_val;\
}
//=================== structs ================
struct http_header {
	char *field_name;
	char *field_value;
	void *next;
};
struct http_connection {
	int socket;
	char *version_string;
	struct addrinfo *address_info;
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
//================== prototypes ===============
int http_request_append_header(struct http_request *request, char *field_name, char *field_value);
//================== public funcitons =============
struct http_connection *http_connect(char *host){
	//setup returned struct
	struct http_connection *connection;
	connection = malloc(sizeof(struct http_connection));
	memset(connection,0,sizeof(struct http_connection));
	if (connection == NULL){
		perror("malloc");
		return NULL;
	}

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
	int status = close(connection->socket);
	if (status < 0){
		perror("close");
	}
	freeaddrinfo(connection->address_info);
	free(connection);
	return 0;
}
int http_send_request(struct http_connection *connection,struct http_request *request){
	//add content length header
	char content_length_buffer[100]; //its a number
	snprintf(content_length_buffer,sizeof(content_length_buffer),"%lu",request->body_size);
	http_request_append_header(request,"content-length",content_length_buffer);

	//setup a buffer for the full request
	char *full_request = NULL;
	long long int full_request_size = 0;

	//add the request line
	int request_line_size = snprintf(NULL,0,"%s %s HTTP/1.1\r\n",request->method,request->url)+1;//get size of request line
	full_request_size += request_line_size;
	full_request = realloc(full_request,full_request_size);
	snprintf(full_request,request_line_size,"%s %s HTTP/1.1\r\n",request->method,request->url);
	
	//add the headers
	struct http_header *node;
	node = request->header;
	for (int node_count = 0;;node_count++){
		if (node == NULL){
			break;
		}
		int header_length = snprintf(NULL,0,"%s: %s\r\n",node->field_name,node->field_value)+1;
		full_request_size += header_length;
		full_request = realloc(full_request,full_request_size);
		char *header_buffer = malloc(header_length);
		snprintf(header_buffer,header_length,"%s: %s\r\n",node->field_name,node->field_value);
		strcat(full_request,header_buffer);
		//next node
		free(header_buffer);
		struct http_header *old_node;
		old_node = node;
		node = old_node->next;
	}

	//mark the start of the body
	// body seperated by \r\n\r\n
	full_request_size += 2;  // \r\n
	full_request_size += request->body_size;
	full_request = realloc(full_request,full_request_size);
	strcat(full_request,"\r\n");
	if (request->body_size > 0){
		memcpy(full_request+full_request_size,request->body,request->body_size);// concatenate the body to the end of the request
	}
	//printf("body_size: %lu\n",request->body_size);
	

	//printf("%s",full_request);
	//send the full request packet
	char *send_buffer = full_request;
	long long int bytes_to_send = full_request_size;
	for (;;){
		int bytes_sent = send(connection->socket,send_buffer,bytes_to_send,0);
		if (bytes_sent < 0){
			perror("send");
			return -1;
		}
		bytes_to_send =- bytes_sent;
		send_buffer += bytes_sent;
		if (bytes_to_send <= 0){
			break;
		}
	}
	free(full_request);

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
	request->body = NULL;
	request->body_size = 0;
	request->header = NULL;
	return request;
}
int http_free_request(struct http_request *request){
	if (request == NULL){
		fprintf(stderr,"http_free_request: request provided is NULL");
		return -1;
	}
	free(request->method);
	free(request->url);
	//free the linked list of headers
	struct http_header *node;
	node = request->header;
	for (;;){
		if (node == NULL){
			break;
		}
		struct http_header *prev_node = node;
		node = prev_node->next;
		//printf("freeing header \"%s: %s\"\n",prev_node->field_name,prev_node->field_value);
		free(prev_node->field_name);
		free(prev_node->field_value);
		free(prev_node);
	}
	free(request->body);
	free(request);
	return 0;
}
static struct http_header *get_header(char *field_name,struct http_header *header){
	if (header == NULL) return NULL;
	if (strcmp(header->field_name,field_name) == 0) return header;
	return get_header(field_name,header->next);
}
//recursive func for freeing linked list
static void print_headers(struct http_header *header){
	if (header == NULL){
		return;
	}else {
		print_headers(header->next);
		printf("%s: %s\n",header->field_name,header->field_value);
		return;
	}
}
static int recursive_free_header_node(struct http_header *header){
	if (header == NULL){
		return 0; //base condition
	} else {
		recursive_free_header_node((struct http_header *)header->next); //recursive step
		free(header->field_name);
		free(header->field_value);
		free(header);
		return 0;
	}
}
char *http_get_header_value(struct http_response *response, char *field_name){
	struct http_header *result = get_header(field_name,response->header);
	if (result == NULL){//dont dereference null pointer
		return NULL;
	}
	return result->field_value;
}
int http_free_response(struct http_response *response){
	recursive_free_header_node(response->header);
	free(response->status_message);
	if (response->body != NULL){
		free(response->body);
	}
	free(response);
	return 0;
}
struct http_response *http_receive_response(struct http_connection *connection){
	//============= prep struct to return back ================
	struct http_response *response = malloc(sizeof(struct http_response));
	//====================== get response line ========================
	int status_code = -1;
	char *response_line = malloc(1);
	if (response_line == NULL){
		perror("malloc");
		return NULL;
	}
	char *response_line_head;
	size_t response_line_size = 1;
	for (;;){
		int result = recv(connection->socket,response_line+response_line_size-1,1,0);
		if (result < 0){
			perror("recv");
			return NULL;
		}
		response_line_size += result;
		response_line = realloc(response_line,response_line_size);
		if (response_line_size > 3 && strncmp(response_line+response_line_size-3,"\r\n",2) == 0){
			//take the 
			response_line[response_line_size-3] = '\0';
			response_line_size -= 3;
			break;
		}
	}
	// -------- break down line ------
	// status
	for (response_line_head = response_line;;response_line_head++){ //response line SIZE << strlen +1
		if (strncmp(response_line_head," ",1) == 0){
			response_line_head += 1;
			break;
		}
	}
	char *temp_status_message = NULL;
	status_code = strtol(response_line_head,&temp_status_message,10);//strtol will give the end of the numbers pointer
	// status message
	size_t status_message_size = strlen(temp_status_message);
	char *status_message = NULL;
	status_message = malloc(status_message_size);
	snprintf(status_message,status_message_size,"%s",temp_status_message+1);
	//clean up
	free(response_line);
	response->status_code = status_code;
	response->status_message = status_message;
	//========================== receive the headers ===================
	struct http_header **next_header = &(response->header);
	char *header_line = NULL;
	size_t header_line_size = 0;
	//for each header in the response
	for (;;){

		//for each char in the header
		header_line = malloc(1);
		header_line[0] = '\0';
		header_line_size = 1;
		for (;;){
			int result = recv(connection->socket,header_line+header_line_size-1,1,0);
			if (result < 0){
				perror("recv");
				return NULL;
			}
			header_line_size += result;
			header_line = realloc(header_line,header_line_size);
			header_line[header_line_size-1] = '\0';
			if (header_line_size >= 3 && strncmp(header_line+header_line_size-3,"\r\n",2) == 0){
				//cut of /r/n
				header_line[header_line_size-3] = '\0';
				//start on the next header
				break;
			}
		}
		//process the header
		if (strlen(header_line) == 0){
			free(header_line);
			break;//end of headers
		}
		//printf("%s\n",header_line);
		//allocate the header
		*next_header = malloc(sizeof(struct http_header));
		//find the ':' and split the string
		char *field_name = header_line;
		char *field_value = strchr(field_name,':');
		field_value[0] = '\0';
		field_value++;
		(*next_header)->field_name = malloc(strlen(field_name+1));
		(*next_header)->field_value = malloc(strlen(field_value)+1);
		//http standard dictates field names are case INsensitive
		for (int i = 0;field_name[i] != '\0';i++){
			field_name[i] = tolower(field_name[i]);
		}
		// strip trailing whitespace
		for (int i = 0;field_value[i] != '\0';i++){
			if (isspace(field_value[i]) != 0){
				field_value++;
				i--;
			}else{
				break;
			}
		}
		strcpy((*next_header)->field_name,field_name);
		strcpy((*next_header)->field_value,field_value);

		free(header_line);
		(*next_header)->next = NULL;
		next_header = (struct http_header **)(&(*next_header)->next);
	}
	//print_headers(response->header);
	//================================== receive the body ======================
	//printf("transfer-encoding = %s\n",http_get_header_value(response,"transfer-encoding"));
	response->body = NULL;
	response->body_size = 0;
	if (http_get_header_value(response,"transfer-encoding") != NULL && strcmp(http_get_header_value(response,"transfer-encoding"),"chunked") == 0){
		//=================== decode chunked body =====================
		response->body_size = 0;
		//setup body buffer
		char *response_body = malloc(1);
		size_t response_body_size = 1;
		for (;;){ 
			char *chunk_size_buffer = malloc(1);
			size_t chunk_size_buffer_size = 1;
			for (;;){
				//receive the chunk size
				int result = recv(connection->socket,chunk_size_buffer+chunk_size_buffer_size-1,1,0);
				if (result < 0){
					perror("recv");
					fprintf(stderr,"could not get chunk size\n");
					return NULL;
				}
				chunk_size_buffer_size += result;
				chunk_size_buffer = realloc(chunk_size_buffer,chunk_size_buffer_size);
				chunk_size_buffer[chunk_size_buffer_size-1] = '\0';
				if (chunk_size_buffer_size > 2 && strcmp(chunk_size_buffer+chunk_size_buffer_size-3,"\r\n") == 0){
					break;
				}
			}
			long long int chunk_size = strtol(chunk_size_buffer,NULL,16);
			//printf("getting chunk of size %lld from string %s\n",chunk_size,chunk_size_buffer);
			response->body_size += chunk_size;
			free(chunk_size_buffer);
			//last chunk
			if (chunk_size == 0) break;
			char *chunk = malloc(sizeof(char)*(chunk_size+1));
			if (chunk == NULL){
				perror("malloc");
				return NULL;
			}
			//recvall the chunk
			for (int chunk_index = 0;chunk_index < chunk_size-1;){
				int result = recv(connection->socket,chunk+chunk_index,chunk_size-chunk_index,0);
			//	printf("received %d bytes\n",result);
				if (result < 0){
					fprintf(stderr,"could not receive chunk\n");
					perror("recv");
					return NULL;
				}
				chunk_index += result;
			}
			//strip trailing \r\n
			char end_buffer[2];
			int result = recv(connection->socket,end_buffer,sizeof(end_buffer),0);
			if (result < 0){
				perror("recv");
				return NULL;
			}
			//end of chunk
			response_body_size += chunk_size;
			response_body = realloc(response_body,response_body_size);
			memcpy(response_body+response_body_size-1-chunk_size,chunk,chunk_size);
			response_body[response_body_size-1] = '\0';
			
			free(chunk);
		}
		response->body = response_body;
	}else if (http_get_header_value(response,"content-length") != NULL){
		long int content_length = strtol(http_get_header_value(response,"content-length"),NULL,10);
		response->body_size = content_length;
		response->body = malloc(content_length);
		int status = tcp_recvall(connection->socket,response->body,content_length);//tcp.c
		if (status < 0){
			response->body = NULL;
			response->body_size = 0;
		}
		//printf("body[%ld]: %s\n",content_length,response->body);
	}else {
		response->body = NULL;
		response->body_size = 0;
	}
	return response;
}
int http_request_append_header(struct http_request *request, char *field_name, char *field_value){
	//find the tail node
	struct http_header *next_node;
	next_node = request->header;
	struct http_header **new_node_location = &request->header;
	for (;;){
		if (next_node == NULL){
			break;
		}
		struct http_header *node;
		node = next_node;
		next_node = node->next;
		new_node_location = (struct http_header **)&node->next;
	}
	//append new struct
	struct http_header *new_node = malloc(sizeof(struct http_header));
	if (new_node == NULL){
		perror("malloc");
		return -1;
	}
	memset(new_node, 0, sizeof(struct http_header));
	new_node->next = NULL;
	new_node->field_name = malloc(strlen(field_name)+1);
	new_node->field_value = malloc(strlen(field_value)+1);
	snprintf(new_node->field_name,strlen(field_name)+1,"%s",field_name);
	snprintf(new_node->field_value,strlen(field_value)+1,"%s",field_value);
	*new_node_location = new_node;
	return 0;
}
int http_request_add_body(struct http_request *request, char *body, size_t body_size){
	request->body = malloc(body_size);
	request->body_size = body_size;
	memcpy(request->body,body,body_size);
	return 0;
}
