#include "aesdsocket.h"

static struct sigaction sig_action = {0};

// Flag set by SIGTERM and SIGINT to notify functions to stop what they're doing
// and exit.
static bool terminate = false;

void signal_handler(int signo)
{
    if (signo == SIGINT || signo == SIGTERM)
    {
        syslog(LOG_USER, "Caught termination signal. Exiting.");
        terminate = true;
    }
}

int init_signal_handler()
{
    sig_action.sa_handler = signal_handler;
    memset(&sig_action.sa_mask, 1, sizeof(sig_action.sa_mask));
    if (sigaction(SIGINT, &sig_action, NULL) != 0)
    {
        perror("sigaction");
        syslog(LOG_ERR, "sigaction failed for SIGINT.");
        return -1;
    }

    if (sigaction(SIGTERM, &sig_action, NULL) != 0)
    {
        perror("sigaction");
        syslog(LOG_ERR, "sigaction failed for SIGTERM.");
        return -1;
    }
    syslog(LOG_USER, "Signal handler setup succeeded.");
    return 0;
}

int send_file_contents_over_socket(socket_context *socket_context)
{
    ssize_t bytesRead = 0;
    FILE *dataFile = fopen(FILENAME, "r");
    if (!dataFile)
    {
        syslog(LOG_ERR, "Error opening data file.");
        return -1;
    }

    char buffer[BUFFERSIZE];
    while ((bytesRead = fread(buffer, 1, sizeof(buffer), dataFile)))
    {
        if ((send(
                *socket_context->p_accept_connection_fd, buffer, bytesRead, 0)) !=
            bytesRead)
        {
            syslog(
                LOG_ERR, "Error sending file contents over socket connection.");
            return -1;
        }
    }
    fclose(dataFile);
    return 0;
}

int receive(socket_context *p_socket_context)
{
    FILE *dataFile = fopen(FILENAME, "a");
    if (!dataFile)
    {
        syslog(LOG_ERR, "Unable to open file for writing.");
        return -1;
    }

    size_t bytesReceived = 0;
    char buffer[BUFFERSIZE];

    while ((bytesReceived = recv(
                *p_socket_context->p_accept_connection_fd,
                buffer,
                sizeof(buffer),
                0)) > 0)
    {
        fwrite(buffer, 1, bytesReceived, dataFile);
        if (memchr(buffer, '\n', bytesReceived) != NULL)
        {
            break;
        }
    }
    fclose(dataFile);

    // Send back the contents of the file over the socket connection.
    send_file_contents_over_socket(p_socket_context);
    return 0;
}

int accept_connections(socket_context *p_socket_context)
{
    while (1)
    {
        if (terminate)
        {
            // A termination signal was sent. Immediately break out of this loop
            // and exit the function.
            return 0;
        }
        *p_socket_context->p_sock_addr_storage_size =
            sizeof(*p_socket_context->p_sock_addr_storage);
        int acceptedConnectionFd = accept(
            *p_socket_context->p_socket_fd,
            (struct sockaddr *)p_socket_context->p_sock_addr_storage,
            p_socket_context->p_sock_addr_storage_size);
        if (acceptedConnectionFd == -1)
        {
            syslog(LOG_ERR, "Eroor accepting connection.");
            continue;
        }
        else
        {
            p_socket_context->p_accept_connection_fd = &acceptedConnectionFd;
            syslog(LOG_USER, "Accepted connection.");

            receive(p_socket_context);
        }
    }
    return 0;
}

int cleanup(socket_context *pSocketContext)
{
    freeaddrinfo(pSocketContext->p_addr_info);
    pSocketContext->p_addr_info = NULL;
    pSocketContext->p_hints = NULL;
    pSocketContext->p_sock_addr_storage = NULL;
    pSocketContext->p_sock_addr_storage_size = NULL;
    pSocketContext->p_socket_fd = NULL;
    pSocketContext->p_accept_connection_fd = NULL;

    free(pSocketContext);
    remove(FILENAME);
    return 0;
}
