
# `int tcp_recvall(int socket, char *buffer, size_t max)`

## Usage

This function will receive up to `size_t max` bytes from `int socket`. This function expects the buffer you provide it is allocated already.

## Return value

bytes received (positive) on success and -1 on failure

# `int tcp_recv_to_crlf(int socket, char *buffer)`

## Usage

This function is used by providing a socket, and pointer to a `char *` to be allocated by this function. It will receive in 1 byte increments untill it finds a `\r\n`, at which point it will cut the `\r\n` by null terminating the string.

## Return value

Returns size of the buffer(positive) on success and -1 on failure
