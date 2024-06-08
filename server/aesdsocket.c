#include "aesdsocket.h"

int sockfd = 0;
FILE *file_ptr = NULL;
pthread_mutex_t thread_list_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t file_mutex = PTHREAD_MUTEX_INITIALIZER;

size_t get_available_heap_size()
{
    struct mallinfo mi = mallinfo();
    size_t available_heap = mi.fordblks;

    return available_heap;
}

void daemonize()
{
    pid_t pid, sid;

    pid = fork();
    if (pid < 0)
    {
        exit(EXIT_FAILURE);
    }

    if (pid > 0)
    {
        exit(EXIT_SUCCESS); // Parent process exits
    }

    umask(0); // Set file permissions

    sid = setsid(); // Create a new session
    if (sid < 0)
    {
        exit(EXIT_FAILURE);
    }

    // Close standard file descriptors
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
}

void *handle_client_connection(void *arg)
{

    bool ioctrl_tracker = false;
    struct ThreadInfo *info = (struct ThreadInfo *)arg;
    int client_sock = info->client_socket;
    struct sockaddr_in client_addr = info->client_thread_addr;
    // socklen_t sin_thread_size_local = info->sin_thread_size;
    char client_ip[INET_ADDRSTRLEN];
    struct aesd_seekto seek_data_from_server;
    // Receive data and append to file
    file_ptr = fopen(FILE_NAME, "w+");
    if (file_ptr == NULL)
    {
        perror("fopen");
        close(client_sock);
        // return -1;
    }
    pthread_mutex_lock(&file_mutex);
    char *buffer = (char *)malloc(BUFFER_SIZE * sizeof(char));
    memset(buffer, 0, BUFFER_SIZE * sizeof(char));
    size_t available_heap = get_available_heap_size();
    pthread_mutex_unlock(&file_mutex);

    // Log the accepted connection to syslog with client IP address
    inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, INET_ADDRSTRLEN);
    syslog(LOG_INFO, "Accepted connection from %s", client_ip);

    ssize_t total_received = 0;
    ssize_t bytes_received;
    char *ptr = NULL;
    pthread_mutex_lock(&file_mutex);

    while ((bytes_received = recv(client_sock, buffer, BUFFER_SIZE - 1, 0)) > 0)
    {
        if ((total_received > available_heap))
        {
            syslog(
                LOG_INFO,
                "Packet too large for available heap, closing connection");
            fclose(file_ptr);
            close(client_sock);
            free(buffer);
            buffer = NULL;
            return NULL;
        }
        ptr = strchr(buffer, '\n');
        if (ptr != NULL)
        {
            break;
        }
        total_received += bytes_received;
    }

    char *start_of_command = strstr(buffer, "AESDCHAR_IOCSEEKTO:");
    if (start_of_command != NULL)
    {
        ioctrl_tracker = true;
        sscanf(
            start_of_command,
            "AESDCHAR_IOCSEEKTO:%u,%u",
            &seek_data_from_server.write_cmd,
            &seek_data_from_server.write_cmd_offset);
        char *end_of_command = strchr(start_of_command, '\n');
        if (end_of_command != NULL)
        {
            end_of_command++; // move past the newline
        }
        else
        {
            end_of_command =
                start_of_command +
                strlen("AESDCHAR_IOCSEEKTO:"); // just move past the command if
                                               // no newline is found
        }
        char *dest = start_of_command;
        char *src = end_of_command;
        while (*src)
        {
            *dest = *src;
            dest++;
            src++;
        }
        *dest = '\0'; // Null-terminate the string
        total_received -=
            (end_of_command -
             start_of_command); // adjust the total_received count
        *start_of_command = '\0';
        *end_of_command = '\0';
        *src = '\0';
    }

    // if (ioctrl_tracker == false)
    //{
    fprintf(file_ptr, "%s", buffer);
    fseek(file_ptr, 0, SEEK_END);
    long file_size = ftell(file_ptr);
    fseek(file_ptr, 0, SEEK_SET);
    //}
    // Allocate memory for the file contents (+1 for null-terminator)
    char *file_contents = (char *)malloc(file_size + 1);
    if (!file_contents)
    {
        perror("malloc");
        pthread_mutex_unlock(&file_mutex);
        free(buffer);
        buffer = NULL;
        return NULL;
    }
    else
    {
        memset(file_contents, 0, sizeof(file_contents));
    }

    if (ioctrl_tracker == true)
    {
        syslog(
            LOG_INFO,
            "Encoded seek data: write_cmd = %u, write_cmd_offset = %u",
            seek_data_from_server.write_cmd,
            seek_data_from_server.write_cmd_offset);
        int file_descriptor = fileno(file_ptr);
        if (file_descriptor == -1)
        {
            // Handle the error, e.g., print to syslog and exit
            syslog(LOG_ERR, "Error getting file descriptor: %m");
            fclose(file_ptr); // Close the file if it's open
            exit(EXIT_FAILURE);
        }
        // Get the ioctl command number
        unsigned int ioctlCommand = _IOC_NR(AESDCHAR_IOCSEEKTO);
        // Get the ioctl command type
        unsigned int ioctlType = _IOC_TYPE(AESDCHAR_IOCSEEKTO);
        // Log the ioctl command type to syslog
        syslog(LOG_INFO, "IOCTL Command Type: %u", ioctlType);
        // Log the ioctl command number to syslog
        syslog(LOG_INFO, "IOCTL Command Number: %u", ioctlCommand);
        syslog(LOG_INFO, "ioctl command: %ld", AESDCHAR_IOCSEEKTO);
        syslog(LOG_INFO, "launching ioctl call..");
        ioctlCommand = 0;
        ioctlType = 0;
        if (ioctl(
                file_descriptor, AESDCHAR_IOCSEEKTO, &seek_data_from_server) ==
            -1)
        {
            // if(ioctl(file_descriptor, AESDCHAR_IOCSEEKTO,
            // &seek_data_from_server) == -1) {
            syslog(LOG_ERR, "ioctl exploded");
            // syslog(LOG_ERR, "ioctl failed: %s", strerror(errno));
            syslog(
                LOG_ERR, "ioctl failed: %s (errno=%d)", strerror(errno), errno);
        }
        else
        {
            long position = ftell(file_ptr);
            syslog(
                LOG_INFO, "Current file position after ioctl: %ld", position);
            syslog(
                LOG_INFO,
                "ioctrl_tracker true, write_cmd: %u, write_cmd_offset: %u",
                seek_data_from_server.write_cmd,
                seek_data_from_server.write_cmd_offset);
        }
        file_descriptor = 0;
    }
    else
    {
        syslog(
            LOG_INFO,
            "ioctrl_tracker false, write_cmd: %u, write_cmd_offset: %u",
            seek_data_from_server.write_cmd,
            seek_data_from_server.write_cmd_offset);
    }

    size_t bytes_read;
    // while ((bytes_read = fread(buffer, 1, sizeof(buffer), file_ptr)) > 0) {
    while ((bytes_read = fread(file_contents, 1, file_size, file_ptr)) > 0)
    {
        ssize_t bytes_sent = send(client_sock, file_contents, bytes_read, 0);
        // printf("Sent %zu bytes to client.\n", bytes_sent);
        if (bytes_sent == -1)
        {
            perror("send");
            pthread_mutex_unlock(&file_mutex);
            free(buffer);
            buffer = NULL;
            free(file_contents);
            file_contents = NULL;
            break;
        }
    }
    pthread_mutex_unlock(&file_mutex);

    fclose(file_ptr);

    // Log closed connection to syslog
    syslog(LOG_INFO, "Closed connection from %s", client_ip);
    close(client_sock);
    memset(buffer, 0, BUFFER_SIZE * sizeof(char));
    free(buffer);
    buffer = NULL;
    free(file_contents);
    file_contents = NULL;
    info->thread_complete_flag = 1;
    pthread_exit(NULL);
}

void sigint_handler(int signum)
{
    syslog(LOG_INFO, "Caught signal, exiting");
    fclose(file_ptr);
    close(sockfd);
    unlink(FILE_NAME); // Delete the file
    closelog();
    exit(EXIT_SUCCESS);
}