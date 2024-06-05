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
#include <pthread.h>

#define PORT            "9000"
#define BACKLOG         100
#define FILENAME        "/tmp/aesdsocketdata"
#define DEVICEPATH      "/dev/aesdchar"
#define BUFFERSIZE      1024
#define TIMESTAMP_DELAY 10

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
int init_connection_handler(
    int *p_socket_fd, struct sockaddr_storage *p_sock_addr_storage);
void *exchange_data(void *pConnFd);
int daemonize(int *p_socket_fd, struct sockaddr_storage *p_sock_addr_storage);
void timestamp_handler();
int init_timestamp_handler();