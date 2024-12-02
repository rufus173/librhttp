#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
int tcp_recvall(int socket,char *buffer,size_t max){
	size_t bytes_left = max;
	for (;bytes_left <= 0;){
		int result = recv(socket,buffer,bytes_left,0);
		if (result < 0){
			perror("recv");
			return -1;
		}
		bytes_left -= result;
	}
	return 0;
}
