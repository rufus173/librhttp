#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
int tcp_recvall(int socket,char *buffer,size_t max){
	long int bytes_left = max;
	//printf("attempting to get %ld bytes\n",max);
	long int bytes_received = 0;
	for (;bytes_left > 0;){
		int result = recv(socket,buffer+bytes_received,bytes_left,0);
		if (result < 0){
			perror("recv");
			return -1;
		}
		bytes_left -= result;
		bytes_received += result;
	}
	//printf("got %ld bytes\n",bytes_received);
	return 0;
}
long int tcp_recv_to_crlf(int socket,char **buffer){
	long int bytes_received = 0;
	char recv_buffer;
	char *final_buffer = malloc(1);
	if (final_buffer == NULL){
		perror("malloc");
		return -1;
	}
	for (;;){
		int result = recv(socket,&recv_buffer,1,0);
		if (result < 0){
			perror("recv");
			return -1;
		}
		//make the buffer bigger and insert the new byte
		//printf("got %c\n",*recv_buffer);
		bytes_received += result;
		final_buffer = realloc(final_buffer,bytes_received);
		if (final_buffer == NULL){
			perror("realloc");
			return -1;
		}
		final_buffer[bytes_received-1] = recv_buffer;
		if (bytes_received >= 2 && strncmp(final_buffer+bytes_received-2,"\r\n",2) == 0){
			//done receiving, terminate string and cut of crlf
			final_buffer[bytes_received-2] = '\0';
			break;
		}
	}
	buffer[0] = final_buffer;
	return bytes_received-2; //crlf not included
}
