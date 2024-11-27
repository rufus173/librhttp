
# Using the shared library

## Connecting to a server

Use the function `HTTP_connection *http_connect(char *host)` to connect to the host, e.g. `google.com`. This will the return a `HTTP_connection *` to be used in later functions with all the connection info. You MUST disconnect from the server after you are done. This will also free the memory allocated for the connection struct.
Return value of NULL on failure.

## Preparing a request to send

Use the function `HTTP_request *http_generate_request(char *method, char *url)` to initialise and allocate a `HTTP_request *`. This can then be passed to `http_send_request()` to send. You MUST use this function for the creation of this struct as this function allocates and initialises the struct in specific ways. Make sure to free the request after you are done with it.
Returns NULL on failure

## Receiving a response

Use the function `HTTP_response *http_receive_response(HTTP_connection *)` to receive a response from the server after making a request. It will allocate the struct returned, which can be freed after use with `http_free_response()`.
Feel free to examine or pick data out from the struct.
	struct http_response {
		int status_code; //404
		char *status_message; //resource not found
		struct http_header *header; //linked list
		char *body;
	};
Return value NULL on failure.

## Freeing a request

Use the function `int http_free_response(HTTP_response *)` to free the response returned by `http_receive_response()`.
Returns 0 on success and < 0 on failure

## Freeing a response

Use the function `int http_free_request(HTTP_request *)` to free a request allocated by `http_generate_request()`.
Returns 0 on success and < 0 on failure

## Disconnecting from a server

Use the function `int http_disconnect(HTTP_request *)` with an active connection to disconnect. Return value 0 on success and < 0 on failure
