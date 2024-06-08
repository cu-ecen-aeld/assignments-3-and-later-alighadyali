#ifndef _AESDSOCKET_H_
#define _AESDSOCKET_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <syslog.h>
#include <signal.h>
#include <stdbool.h>
#include <getopt.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <malloc.h>
#include <pthread.h>
#include <sys/queue.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include "../aesd-char-driver/aesd_ioctl.h"

#define USE_AESD_CHAR_DEVICE 1

#define BUFFER_SIZE 1000000
#define PORT        9000

#ifdef USE_AESD_CHAR_DEVICE
#define FILE_NAME "/dev/aesdchar"
#else
#define FILE_NAME "/var/tmp/aesdsocketdata"
#endif

extern int sockfd;
extern FILE *file_ptr;

struct ThreadInfo
{
    pthread_t thread_id;
    int client_socket;
    int thread_complete_flag;
    struct sockaddr_in client_thread_addr;
    socklen_t sin_thread_size;
    SLIST_ENTRY(ThreadInfo) entries;
};

extern pthread_mutex_t thread_list_mutex;
extern pthread_mutex_t file_mutex;

size_t get_available_heap_size();

void daemonize();

void *handle_client_connection(void *arg);

void sigint_handler(int signum);

#endif