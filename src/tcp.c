#include <stddef.h>
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
	char *recv_buffer;
	*buffer = malloc(1);
	for (;;){
		int result = recv(socket,recv_buffer,1,0);
		if (result < 0){
			perror("recv");
			return -1;
		}
		bytes_received += result;
		*buffer = realloc(*buffer,bytes_received);
	}
}
