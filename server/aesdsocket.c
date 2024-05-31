#include "aesdsocket.h"

static struct sigaction sig_action = {0};

// Flag set by SIGTERM and SIGINT to notify functions to stop what they're doing
// and exit.
static bool terminate = false;
static pthread_mutex_t fmutex;

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

void timestamp_handler()
{
    while (true)
    {
        pthread_mutex_lock(&fmutex);
        if (terminate)
        {
            pthread_mutex_unlock(&fmutex);
            return;
        }
        FILE *dataFile = fopen(FILENAME, "a");
        if (dataFile == NULL)
        {
            perror("fopen");
            syslog(LOG_ERR, "Failed to open data file.");
            pthread_mutex_unlock(&fmutex);
            sleep(TIMESTAMP_DELAY);
            continue;
        }

        time_t now = time(NULL);
        struct tm *pTimeData = localtime(&now);
        char timestamp[64];
        strftime(
            timestamp,
            sizeof(timestamp),
            "timestamp:%Y-%m-%d %H:%M:%S\n",
            pTimeData);

        fprintf(dataFile, "%s", timestamp);
        fclose(dataFile);
        pthread_mutex_unlock(&fmutex);

        sleep(TIMESTAMP_DELAY);
    }
    return;
}

int init_timestamp_handler()
{
    pthread_t timestamp_thread;
    if (pthread_create(
            &timestamp_thread, NULL, (void *)timestamp_handler, NULL) < 0)
    {
        perror("Timestamp thread failed");
        return -1;
    }
    return 0;
}

int send_file_contents_over_socket(socket_context *socket_context)
{
    ssize_t bytes_read = 0;
    FILE *data_file = fopen(FILENAME, "r");
    if (!data_file)
    {
        syslog(LOG_ERR, "Error opening data file.");
        return -1;
    }

    char buffer[BUFFERSIZE];
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), data_file)))
    {
        if ((send(
                *socket_context->p_accept_connection_fd,
                buffer,
                bytes_read,
                0)) != bytes_read)
        {
            syslog(
                LOG_ERR, "Error sending file contents over socket connection.");
            return -1;
        }
    }
    fclose(data_file);
    return 0;
}

int receive(socket_context *p_socket_context)
{
    pthread_mutex_lock(&fmutex);
    FILE *data_file = fopen(FILENAME, "a");
    if (!data_file)
    {
        syslog(LOG_ERR, "Unable to open file for writing.");
        pthread_mutex_unlock(&fmutex);
        return -1;
    }

    size_t bytes_received = 0;
    char buffer[BUFFERSIZE];

    while ((bytes_received = recv(
                *p_socket_context->p_accept_connection_fd,
                buffer,
                sizeof(buffer),
                0)) > 0)
    {
        fwrite(buffer, 1, bytes_received, data_file);
        if (memchr(buffer, '\n', bytes_received) != NULL)
        {
            break;
        }
    }
    fclose(data_file);

    // Send back the contents of the file over the socket connection.
    send_file_contents_over_socket(p_socket_context);
    pthread_mutex_unlock(&fmutex);
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
        pthread_t thread_id;
        *p_socket_context->p_sock_addr_storage_size =
            sizeof(*p_socket_context->p_sock_addr_storage);
        int accepted_connection_fd = accept(
            *p_socket_context->p_socket_fd,
            (struct sockaddr *)p_socket_context->p_sock_addr_storage,
            p_socket_context->p_sock_addr_storage_size);
        if (accepted_connection_fd == -1)
        {
            syslog(LOG_ERR, "Error accepting connection.");
            continue;
        }
        else
        {
            p_socket_context->p_accept_connection_fd = &accepted_connection_fd;
            syslog(LOG_USER, "Accepted connection.");

            if (pthread_create(
                    &thread_id,
                    NULL,
                    (void *)receive,
                    (socket_context *)p_socket_context) < 0)
            {
                perror("pthread_create");
                syslog(
                    LOG_ERR,
                    "Unable to create thread while accepting connection.");
                return -1;
            }
            pthread_join(thread_id, NULL);
        }
    }
    return 0;
}

int cleanup(socket_context *p_socket_context)
{
    freeaddrinfo(p_socket_context->p_addr_info);
    p_socket_context->p_addr_info = NULL;
    p_socket_context->p_hints = NULL;
    p_socket_context->p_sock_addr_storage = NULL;
    p_socket_context->p_sock_addr_storage_size = NULL;
    p_socket_context->p_socket_fd = NULL;
    p_socket_context->p_accept_connection_fd = NULL;

    free(p_socket_context);
    remove(FILENAME);
    return 0;
}
