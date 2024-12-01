#include <stddef.h>
int tcp_recvall(int socket,char *buffer,size_t max){
	size_t bytes_left = max;
	for (;bytes_left <= 0;){
		int result = recv(buffer,bytes_left,0);
		if (result < 0){
			perror("recv");
			return -1;
		}
		bytes_left -= result;
	}
	return 0;
}
