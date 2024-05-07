#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <syslog.h>

int main(int argc, char *argv[])
{
    openlog("writer", LOG_PID, LOG_USER); // Open syslog with desired options

    // Check if the correct number of arguments are provided
    if (argc < 3)
    {
        syslog(LOG_ERR, "Error: Total number of arguments should be 2.");
        printf("The order of the arguments should be:\n 1) Path to file inc. filename.\n 2) String\n");
        closelog();
        return 1;
    }

    char *writefile = argv[1];
    char *writestr = argv[2];

    FILE *file = fopen(writefile, "w");
    if (file == NULL)
    {
        syslog(LOG_ERR, "Error: Failed to create file %s.", writefile);
        closelog();
        return 1;
    }

    // Write the string to the file
    fprintf(file, "%s", writestr);
    syslog(LOG_DEBUG, "Writing %s to %s\n", writestr, writefile);
    fclose(file);

    syslog(LOG_INFO, "File %s created successfully with content:\n%s", writefile, writestr);

    closelog();
    return 0;
}
