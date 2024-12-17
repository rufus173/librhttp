#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "http.h"

//open ssl stuff
#include <openssl/ssl.h>
#include <openssl/bio.h>
//#include <openssl/applink.h>
#include <openssl/err.h>

static void init_openssl(){
	SSL_load_error_strings();
	SSL_library_init();
	OpenSSL_add_all_algorithms();
}
static void cleanup_ssl(){
	ERR_free_strings();
	EVP_cleanup();
}
//normal stuff
static int tcp_send(struct http_connection *connection, void *buffer, size_t length, int flags){
	if (connection->use_ssl == 0){
		return send(connection->socket,buffer,length,flags);
	}
	if (connection->use_ssl == 1){
		size_t written = 0;
		int result = SSL_write_ex(connection->ssl_connection, buffer, length, &written);
		if (result < 1) return -1;
		return (int)written;
	}
	fprintf(stderr,"connection struct error: did you initialise it?\n");
	return -1;
}
int tcp_sendall(struct http_connection *connection,void *buffer,size_t length, int flags){
	printf("length: %lu\n",length);
	printf("%s",(char *)buffer);
	long long int bytes_to_send = length;
	char *send_buffer = buffer;
	for (;;){
		int bytes_sent = tcp_send(connection,send_buffer,bytes_to_send,0);
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
	return 0;
}
static int tcp_recv(struct http_connection *connection, void *buf, size_t len, int flags){
	if (connection->use_ssl == 0){
		return recv(connection->socket,buf,len,flags);
	}
	if (connection->use_ssl == 1){
		size_t read = 0;
		int result = SSL_read_ex(connection->ssl_connection, buf, len, &read);
		if (result < 1) return -1;
		return read;
	}
	fprintf(stderr,"connection struct error: did you initialise it?\n");
	return -1;
}
int tcp_connect_socket(struct http_connection *connection, char *address,char *port){
	int return_status = 0;
	//resolve a hostname or url
	struct addrinfo hints, *address_info;
	memset(&hints,0,sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	int result = getaddrinfo(address,port,&hints,&address_info);
	if (result < 0){
		fprintf(stderr,"getaddrinfo: %s\n",gai_strerror(result));
		return -1;
	}
	connection->address_info = address_info;

	//create a socket
	int sock = socket(AF_INET,SOCK_STREAM,0);
	if (sock < 0){
		perror("socket");
		freeaddrinfo(connection->address_info);
		connection->address_info = NULL;
		return -1;
	}
	connection->socket = sock;

	//connect
	result = connect(sock,address_info->ai_addr,address_info->ai_addrlen);
	if (result < 0){
		perror("connect");
		freeaddrinfo(connection->address_info);
		return -1;
	}
	if (connection->use_ssl == 1){
		static int ssl_initialised = 0;
		if (ssl_initialised == 0){
			init_openssl();
			atexit(cleanup_ssl);
			ssl_initialised = 1;
		}
		//enable ssl on socket
		connection->ssl_context = SSL_CTX_new(TLS_client_method());
		if (connection->ssl_context == NULL){
			fprintf(stderr,"couldnt initialise new ssl context.\n");
			freeaddrinfo(connection->address_info);
			return -1;
		}
		connection->ssl_connection = SSL_new(connection->ssl_context);
		if (connection->ssl_connection == NULL){
			fprintf(stderr,"Could not create an ssl connection.\n");
			freeaddrinfo(connection->address_info);
			SSL_CTX_free(connection->ssl_context);
			return -1;
		}
		//create a bio object and associate with ssl connection
		BIO *bio;
		bio = BIO_new(BIO_s_socket());
		if (bio == NULL){
			fprintf(stderr,"failure to initialise bio object.\n");
			return -1;
		}
		BIO_set_fd(bio, connection->socket, BIO_CLOSE); //close socket when bio freed
		SSL_set_bio(connection->ssl_connection,bio,bio);

		//perform handshake
		int result = SSL_connect(connection->ssl_connection);
		if (result < 1){
			fprintf(stderr,"failed handshake with code %ld.\n",SSL_get_verify_result(connection->ssl_connection));
			if (SSL_get_verify_result(connection->ssl_connection) != X509_V_OK){
				fprintf(stderr,"verification error: %s\n",X509_verify_cert_error_string(SSL_get_verify_result(connection->ssl_connection)));
			}
			freeaddrinfo(connection->address_info);
			SSL_free(connection->ssl_connection);
			SSL_CTX_free(connection->ssl_context);
			return -1;
		}
	}
	return return_status;
}
int tcp_close_socket(struct http_connection *connection){
	freeaddrinfo(connection->address_info);
	int status = close(connection->socket);
	if (status < 0){
		perror("close");
		return -1;
	}
	if(connection->use_ssl == 1){
		SSL_shutdown(connection->ssl_connection);
		SSL_free(connection->ssl_connection);
		SSL_CTX_free(connection->ssl_context);
	}
	return 0;
}
int tcp_recvall(struct http_connection *connection,char *buffer,size_t max){
	long int bytes_left = max;
	//printf("attempting to get %ld bytes\n",max);
	long int bytes_received = 0;
	for (;bytes_left > 0;){
		int result = tcp_recv(connection,buffer+bytes_received,bytes_left,0);
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
long int tcp_recv_to_crlf(struct http_connection *connection,char **buffer){
	long int bytes_received = 0;
	char recv_buffer;
	char *final_buffer = malloc(1);
	if (final_buffer == NULL){
		perror("malloc");
		return -1;
	}
	for (;;){
		int result = tcp_recv(connection,&recv_buffer,1,0);
		if (result < 0){
			perror("recv");
			free(final_buffer);
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
