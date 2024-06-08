#include "aesdsocket.h"

SLIST_HEAD(ThreadList, ThreadInfo) thread_list = SLIST_HEAD_INITIALIZER(thread_list);

int main(int argc, char *argv[])
{
    struct sockaddr_in server_addr, client_addr;

    openlog("aesdsocket_server", LOG_CONS | LOG_PID, LOG_USER);

    bool daemon_mode = false;

    // Parse command-line arguments
    int opt;
    while ((opt = getopt(argc, argv, "d")) != -1)
    {
        switch (opt)
        {
        case 'd':
            daemon_mode = true;
            break;
        default:
            fprintf(stderr, "Usage: %s [-d]\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    // Daemonize if in daemon mode
    if (daemon_mode)
    {
        daemonize();
    }

    // Create a socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    int enable = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
    {
        perror("setsockopt(SO_REUSEADDR) failed");
    }

    // Set up the server address structure
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind the socket to the specified PORT
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) ==
        -1)
    {
        perror("bind");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(sockfd, 5) == -1)
    {
        perror("listen");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    socklen_t client_addr_len = sizeof(client_addr);

    signal(SIGINT, sigint_handler);
    signal(SIGTERM, sigint_handler);

    while (1)
    {
        // Accept a connection
        int client_sock =
            accept(sockfd, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_sock == -1)
        {
            perror("accept");
            continue;
        }

        struct ThreadInfo *info =
            (struct ThreadInfo *)malloc(sizeof(struct ThreadInfo));
        if (info == NULL)
        {
            perror("Thread memory allocation error");
            close(client_sock);
            continue;
        }
        info->client_socket = client_sock;
        info->sin_thread_size = sizeof(client_addr);
        info->client_thread_addr = client_addr;
        info->thread_complete_flag = 0;

        pthread_create(&info->thread_id, NULL, handle_client_connection, info);

        // Insert the thread's info into the linked list
        pthread_mutex_lock(&thread_list_mutex);
        SLIST_INSERT_HEAD(&thread_list, info, entries);
        pthread_mutex_unlock(&thread_list_mutex);

        // cleanup_threads(timer_thread);
    }

    // The signal handler will handle the cleanup

    return 0;
}