#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define PORT       "9000"
#define BACKLOG    10
#define FILENAME   "/tmp/aesdsocketdata"
#define BUFFERSIZE 1024

// pointers to file descriptors, structs and vars for socket management.
typedef struct
{
    struct sockaddr_storage *p_sock_addr_storage;
    socklen_t *p_sock_addr_storage_size;
    struct addrinfo *p_hints;
    struct addrinfo *p_addr_info;
    int *p_socket_fd;
    int *p_accept_connection_fd;
} socket_context;

void signal_handler(int signo);
int init_signal_handler();
int send_file_contents_over_socket(socket_context *socket_context);
int accept_connections(socket_context *p_socket_context);
int receive(socket_context *p_socket_context);
int cleanup(socket_context *p_socket_context);