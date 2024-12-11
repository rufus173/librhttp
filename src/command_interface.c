#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include "command_interface.h"
#include "http.h"
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
		int result = process_command(argc,argv);

		//clean up
		for (int i = 0; i < argc;i++) free(argv[i]);
		free(argv);
	}
	return 0;
}
int process_command(int argc, char **argv){
	//====================== static env variables ===================
	static uint32_t flags = 0; // 0 0 0 0 0 0 0              0
	static HTTP_connection *connection = NULL;
	static HTTP_request *request = NULL;
	
	if (argc < 1) return 0;
	//----------------------- process set command ---------------------
	if (strcmp(argv[0],"set") == 0){
		if (argc != 2){
			printf("Error: set requires 1 arg, and %d given.\n",argc-1);
			return ERR_ARG_COUNT;
		}
		if (strcmp(argv[1],"ssl") == 0) flags |= F_SSL; //set flag to 1
		if (strcmp(argv[1],"nossl") == 0) flags &= ~(uint32_t)F_SSL; //set flag to 0
		return 0;
	}
	//---------------------- connect and disconnect -------------------
	if (strcmp(argv[0],"connect") == 0){
		if ((flags & (uint32_t)F_CONNECTED) > 0){
			printf("Already connected.\n");
			return ERR_CANNOT_PROCESS;
		}
		char *port = NULL;
		if (argc < 2){
			printf("Error: expected 1-2 args but got %d.\n",argc-1);
			return ERR_ARG_COUNT;
		}
		if (argc < 3){
			port = "80";
		}else{
			port = argv[2];
		}
		printf("Connecting...\n");
		connection = http_connect(argv[1],port);
		if (connection == NULL){
			printf("Error: could not connect.\n");
			return ERR_FAILURE;
		}
		flags |= F_CONNECTED;
		printf("connected.\n");
		return 0;
	}
	if (strcmp(argv[0],"disconnect") == 0){ 
		if ((flags & (uint32_t)F_CONNECTED) == 0){
			printf("Not connected.\n");
			return ERR_CANNOT_PROCESS;
		}
		//check if connected
		printf("Disconnecting...\n");
		int result = http_disconnect(connection);
		if (result < 0){
			printf("Failed to disconnect.\n");
			return ERR_FAILURE;
		}
		flags &= ~(uint32_t)F_CONNECTED;
		printf("Disconnected.\n");
		return 0;
	}
	
	//---------------- new request -----------------
	if (strcmp(argv[0],"mkrequest") == 0){
		if ((flags & (uint32_t)F_REQUEST_ALLOCATED) > 0){//free any existing request
			printf("Freeing previous request...\n");
			int status = http_free_request(request);
			if (status < 0){
				printf("Error: could not free request.\n");
				return ERR_CANNOT_PROCESS;
			}
			printf("Done.\n");
		}
		if (argc != 3){
			printf("Error: expected 2 arguments.\n");
			return ERR_ARG_COUNT;
		}
		printf("Generating request...\n");
		request = http_generate_request(argv[1],argv[2]);
		if (request == NULL){
			printf("Error: could not generate request.\n");
			return ERR_CANNOT_PROCESS;
		}
		flags |= F_REQUEST_ALLOCATED;
		printf("Success!\n");
		return 0;
	}
	//---------------- send command ----------------
	if (strcmp(argv[0],"send") == 0){
		if ((flags & F_CONNECTED) == 0){
			printf("Not connected.\n");
			return ERR_CANNOT_PROCESS;
		}
		if ((flags & F_REQUEST_ALLOCATED) == 0){
			printf("No request to send.\n");
			return ERR_CANNOT_PROCESS;
		}
		printf("Sending...\n");
		http_send_request(connection,request);
		HTTP_response* response = http_receive_response(connection);
		if (response == NULL){
			printf("Failed to get response.\n");
			return ERR_FAILURE;
		}
		if (response->body == NULL){
			printf("(no body)\n");
			http_free_response(response);
			return 0;
		}
		printf("======== body =======%s\n======== body end =======\n",response->body);
		http_free_response(response);
		return 0;
		
	}

	//---------------- help command ----------------
	if (strcmp(argv[0],"help") == 0){
		printf("Available commands (\"[optional]\", \"<mandatory>\")\n");
		printf("help - display help about available commands\n");
		printf("connect <address> [port] - connect to a server\n");
		printf("disconnect - disconnects active connection\n");
		printf("mkrequest <type e.g. PUT> <path> - makes a request ready to send\n");
		printf("send - sends request to connection and prints response\n");
		return 0;
	}

	//if we get here without returning, the command wasnt found.
	printf("Command \"%s\" not found.\n",argv[0]);
	return ERR_NOT_FOUND -5;
}
