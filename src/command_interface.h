//---- errors ---
#define ERR_FAILURE -1
#define ERR_SYNTAX -2
#define ERR_ARG_COUNT -3
#define ERR_ARG -4
#define ERR_NOT_FOUND -5
#define ERR_CANNOT_PROCESS -6
//---- flags ----
#define F_SSL 1
#define F_CONNECTED 2
#define F_REQUEST_ALLOCATED 4

int process_command(int argc, char **argv);
