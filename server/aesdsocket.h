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
#include "../aesd-char-driver/aesd_ioctl.h"

#define PORT            "9000"
#define BACKLOG         100
#define FILENAME        "/var/tmp/aesdsocketdata"
#define DEVICEPATH      "/dev/aesdchar"
#define BUFFERSIZE      1024
#define TIMESTAMP_DELAY 10

void signal_handler(int signo);
int init_signal_handler();
int init_connection_handler(
    int *p_socket_fd, struct sockaddr_storage *p_sock_addr_storage);
void *exchange_data(void *pConnFd);
int daemonize(int *p_socket_fd, struct sockaddr_storage *p_sock_addr_storage);
void timestamp_handler();
int init_timestamp_handler();
void send_data(int connectionFd, FILE *data_file);