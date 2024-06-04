#include "aesdsocket.h"

int main(int argc, char *argv[])
{
    openlog(NULL, 0, LOG_USER);
    struct sockaddr_storage sock_addr_storage;
    struct addrinfo hints, *p_addr_info;
    int socketFd;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

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
        freeaddrinfo(p_addr_info);
        syslog(LOG_ERR, "Error creating socket");
        exit(EXIT_FAILURE);
    }

    if (bind(socketFd, p_addr_info->ai_addr, p_addr_info->ai_addrlen) == -1)
    {
        syslog(LOG_ERR, "Could not bind socket to address.");
        freeaddrinfo(p_addr_info);
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(p_addr_info);

    if (listen(socketFd, BACKLOG) == -1)
    {
        syslog(LOG_ERR, "Could not invoke listen on socket.");
    }

    if (runAsDaemon)
    {
        daemonize(&socketFd, &sock_addr_storage);
    }

    // Running interactively (not as a daemon).
    syslog(LOG_USER, "Running in interactive mode.");

    // Begin accepting connections and data.
    accept_connections(&socketFd, &sock_addr_storage);
    syslog(LOG_USER, "Closing connections and cleaning up.");

    if (socketFd != -1)
    {
        close(socketFd);
    }
    closelog();
    exit(EXIT_SUCCESS);
}