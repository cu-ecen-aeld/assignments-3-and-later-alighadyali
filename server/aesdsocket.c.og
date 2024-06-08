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
        FILE *dataFile = fopen(DEVICEPATH, "w");
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

void send_data(int connectionFd, FILE *data_file)
{
    ssize_t bytesRead = 0;
    char buffer[BUFFERSIZE];

    while ((bytesRead = read(fileno(data_file), buffer, 1)) > 0)
    {
        if ((send(connectionFd, buffer, bytesRead, 0)) != bytesRead)
        {
            syslog(
                LOG_ERR, "Error sending file contents over socket connection.");
            return;
        }
    }
}

void *exchange_data(void *pConnFd)
{
    int connectionFd = *((int *)pConnFd);
    pthread_mutex_lock(&fmutex);
    FILE *data_file = fopen(DEVICEPATH, "w");
    if (!data_file)
    {
        syslog(LOG_ERR, "Unable to open file for writing.");
        pthread_mutex_unlock(&fmutex);
        return 0;
    }

    size_t bytes_received = 0;
    char buffer[BUFFERSIZE];

    // while ((bytesReceived = recv(connectionFd, buffer, sizeof(buffer), 0)) >
    // 0)
    // {
    //     fwrite(buffer, 1, bytesReceived, dataFile);
    //     if (memchr(buffer, '\n', bytesReceived) != NULL)
    //     {
    //         break;
    //     }
    // }
    // fclose(dataFile);

    // // Send back the contents of the file to the client.
    // ssize_t bytesRead = 0;
    // dataFile = fopen(DEVICEPATH, "r");
    // if (!dataFile)
    // {
    //     syslog(LOG_ERR, "Error opening data file.");
    //     return 0;
    // }

    // // Reset buffer and start sending.
    // memset(buffer, 0, sizeof(buffer));
    // while ((bytesRead = fread(buffer, 1, sizeof(buffer), dataFile)))
    // {
    //     if ((send(connectionFd, buffer, bytesRead, 0)) != bytesRead)
    //     {
    //         syslog(
    //             LOG_ERR, "Error sending file contents over socket
    //             connection.");
    //         return 0;
    //     }
    // }

    while ((bytes_received = recv(connectionFd, buffer, sizeof(buffer), 0)) > 0)
    {
        if (memchr(buffer, '\n', bytes_received) != NULL)
        {
            if (strstr(buffer, "AESDCHAR_IOCSEEKTO"))
            {
                syslog(LOG_USER, "Found ioctl command");
                struct aesd_seekto cmd;
                sscanf(
                    buffer,
                    "AESDCHAR_IOCSEEKTO:%u,%u",
                    &cmd.write_cmd,
                    &cmd.write_cmd_offset);
                syslog(
                    LOG_DEBUG,
                    "Command %u,%u",
                    cmd.write_cmd,
                    cmd.write_cmd_offset);
                ioctl(fileno(data_file), AESDCHAR_IOCSEEKTO, &cmd);
                send_data(connectionFd, data_file);
                break;
            }
            // If not ioctl command is found, write to file, set fpos to 0, and
            // send the contents back to the client.
            write(fileno(data_file), buffer, bytes_received);
            rewind(data_file);
            send_data(connectionFd, data_file);
            break;
        }
    }

    fclose(data_file);
    pthread_mutex_unlock(&fmutex);
    return 0;
}

int init_connection_handler(
    int *p_socket_fd, struct sockaddr_storage *p_sock_addr_storage)
{
    socklen_t sockAddrStorageSize = sizeof(*p_sock_addr_storage);
    while (1)
    {
        if (terminate)
        {
            // A termination signal was sent. Immediately break out of this loop
            // and exit the function.
            return 0;
        }

        pthread_t threadId;
        int connectionFd = accept(
            *p_socket_fd,
            (struct sockaddr *)p_sock_addr_storage,
            &sockAddrStorageSize);
        if (connectionFd == -1)
        {
            syslog(LOG_ERR, "Error accepting connection.");
            continue;
        }
        else
        {
            syslog(LOG_USER, "Accepted connection.");

            if (pthread_create(
                    &threadId, NULL, exchange_data, (void *)&connectionFd) < 0)
            {
                perror("pthread_create");
                syslog(
                    LOG_ERR,
                    "Unable to create thread while accepting connection.");
                return -1;
            }
            pthread_join(threadId, NULL);
        }
    }
    return 0;
}

int daemonize(int *p_socket_fd, struct sockaddr_storage *p_sock_addr_storage)
{
    pid_t pid = fork();
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
    int nullFd = open("/dev/null", O_WRONLY | O_CREAT, 0666);
    dup2(nullFd, 1);

    // Start the timestamp thread.
    // initTimestampHandler();

    // Begin accepting connections and exchanging data.
    init_connection_handler(p_socket_fd, p_sock_addr_storage);

    // After the program is done accepting connections (i.e. after a term signal
    // is received, start cleaning up.
    syslog(LOG_USER, "Closing connections and cleaning up.");
    remove(FILENAME);

    if (nullFd != -1)
    {
        close(nullFd);
    }

    if (*p_socket_fd != -1)
    {
        close(*p_socket_fd);
    }

    closelog();
    exit(EXIT_SUCCESS);
}