#include "aesdsocket.h"

int main(int argc, char *argv[])
{
    socket_context *p_socket_context = malloc(sizeof(socket_context));
    struct sockaddr_storage sock_addr_storage;
    socklen_t sock_addr_storage_size;
    struct addrinfo hints, *p_addr_info;
    int socketFd;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    pid_t pid;

    bool runAsDaemon = false;
    openlog(NULL, 0, LOG_USER);

    for (int i = 1; i < argc; i++)
    {
        if (strstr(argv[i], "-d"))
        {
            runAsDaemon = true;
            break;
        }
    }

    // Initialize the signal handler.
    if (init_signal_handler() == -1)
    {
        return -1;
    }

    // Used to get network and host information for socket binding and
    // listening.
    getaddrinfo(NULL, PORT, &hints, &p_addr_info);

    socketFd = socket(
        p_addr_info->ai_family,
        p_addr_info->ai_socktype,
        p_addr_info->ai_protocol);
    if (socketFd == -1)
    {
        free(p_socket_context);
        freeaddrinfo(p_addr_info);
        syslog(LOG_ERR, "Error creating socket");
        exit(EXIT_FAILURE);
    }

    int sockOpt = 1;
    if (setsockopt(
            socketFd,
            SOL_SOCKET,
            SO_REUSEADDR | SO_REUSEPORT,
            &sockOpt,
            sizeof(sockOpt)))
    {
        syslog(LOG_ERR, "Could not set socket options.");
        free(p_socket_context);
        freeaddrinfo(p_addr_info);
        exit(EXIT_FAILURE);
    }

    if (bind(socketFd, p_addr_info->ai_addr, p_addr_info->ai_addrlen) == -1)
    {
        syslog(LOG_ERR, "Could not bind socket to address.");
        free(p_socket_context);
        freeaddrinfo(p_addr_info);
        exit(EXIT_FAILURE);
    }

    if (listen(socketFd, BACKLOG) == -1)
    {
        syslog(LOG_ERR, "Could not invoke listen on socket.");
    }
    sock_addr_storage_size = sizeof(sock_addr_storage);

    p_socket_context->p_socket_fd = &socketFd;
    p_socket_context->p_sock_addr_storage = &sock_addr_storage;
    p_socket_context->p_sock_addr_storage_size = &sock_addr_storage_size;
    p_socket_context->p_hints = &hints;
    p_socket_context->p_addr_info = p_addr_info;

    if (runAsDaemon)
    {
        pid = fork();
        if (pid == -1)
        {
            syslog(LOG_ERR, "Unable to fork.");
            perror("fork");
            return -1;
        }
        else if (pid != 0)
        {
            syslog(LOG_USER, "Running in daemon mode.");
            exit(EXIT_SUCCESS);
        }

        // Create new session and process group.
        if (setsid() == -1)
        {
            syslog(LOG_ERR, "setsid failed after fark.");
            perror("setsid");
            return -1;
        }

        // CHILD PROCESS STARTED
        // Set the working directory of child processto the root directory.
        if (chdir("/") == -1)
        {
            syslog(LOG_ERR, "chdir failed after fork.");
            perror("chdir");
            return -1;
        }

        // Redirect fds to /dev/null
        int fd = open("/dev/null", O_WRONLY | O_CREAT, 0666);
        dup2(fd, 1);
        init_timestamp_handler();

        // Begin accepting connections and data.
        accept_connections(p_socket_context);
        syslog(LOG_USER, "Closing connections and cleaning up.");
        cleanup(p_socket_context);

        if (fd != -1)
        {
            close(fd);
        }

        if (socketFd != -1)
        {
            close(socketFd);
        }

        closelog();
        exit(EXIT_SUCCESS);
    }

    // Running interactively (not as a daemon).
    syslog(LOG_USER, "Running in interactive mode.");

    init_timestamp_handler();
    // Begin accepting connections and data.
    accept_connections(p_socket_context);
    syslog(LOG_USER, "Closing connections and cleaning up.");
    cleanup(p_socket_context);
    if (socketFd != -1)
    {
        close(socketFd);
    }
    closelog();
    exit(EXIT_SUCCESS);
}