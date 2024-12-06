#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "http.h"
int command_line_mode();
int process_command(int argc, char **argv){
	//                                        print headers  ssl
	static uint32_t flags = 0; // 0 0 0 0 0 0 0              0
	
	if (argc < 1) return 0;
	//process set command
	if (strcmp(argv[0],"set") == 0){
		if (strcmp(argv[1],"ssl") == 0) flags |= 1; //set lsb to 1
		if (strcmp(argv[1],"nossl") == 0) flags &= ~(uint32_t)1; //set lsb tp 0
	}
}
int main(int argc, char **argv){
	if (argc == 1){
		return command_line_mode();
	}
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
int command_line_mode(){
	//======================= logo ===========================
	printf("\
		____\n\
    _______  __/ __/_  _______\n\
   / ___/ / / / /_/ / / / ___/  ______\n\
  / /  / /_/ / __/ /_/ (__  )  /_____/\n\
 /_/   \\__,_/_/  \\__,_/____/\n\
		______\n\
    _________  / __/ /__      ______ _________\n\
   / ___/ __ \\/ /_/ __/ | /| / / __ `/ ___/ _ \\\n\
  (__  ) /_/ / __/ /_ | |/ |/ / /_/ / /  /  __/\n\
 /____/\\____/_/  \\__/ |__/|__/\\__,_/_/   \\___/\n");
	printf("Welcome to the rufus requests command line. Try \"help\" to start.\n");
	
	//======================== mainloop =======================
	for (;;){
		//display prompt
		printf("(rufus requests)");
		//---------------- get user input -----------
		char *command = NULL;
		long int command_size = 1;
		for (;;){
			//quit when ctrl-d pressed
			if (feof(stdin)){
				printf("\033[2K\r[quit]\n"); //clear line -> carrage return -> print quit
				return 0;
			}
			char input_buffer[1];
			int result = fread(input_buffer,1,1,stdin);
			if (result < 0){
				perror("fread");
				return 1;
			}
			command_size++;
			command = realloc(command,command_size);
			command[command_size-2] = *input_buffer;
			command[command_size-1] = '\0';
			if (command[command_size-2] == '\n'){
				break;
			}
		}
		//-------------- preprocess command -------------
		int argc = 0;
		char **argv = NULL;
		int arglen = 0;
		int offset = 0;
		int start_offset = 0;
		for (offset = 0;;offset++){
			if (isspace(command[offset])){
				argc++;
				//put verb into argv
				argv = realloc(argv,(argc)*sizeof(char *));
				argv[argc-1] = malloc(arglen+1);
				memcpy(argv[argc-1],command+start_offset,arglen);
				argv[argc-1][arglen] = '\0';
				
				//stop if end of line
				if (command[offset] == '\n') break; //end of line
 				
				//move to next non whitespace
				for(;isspace(command[offset]);offset++);
				arglen = 0;
				start_offset = offset;
			}
			arglen++;
		}

		//clean up
		free(command);

		//(post pre)process arguments
		for (int i = 0; i < argc; i++){
			printf("%s\n",argv[i]);
		}
		//int result = process_command(argc,argv);

		//clean up
		for (int i = 0; i < argc;i++) free(argv[i]);
		free(argv);
	}
	return 0;
}
