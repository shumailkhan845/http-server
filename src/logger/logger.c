#include "logger.h"

#include <stdio.h>
#include <stdlib.h>

void log_access(struct log_entry *log)
{
    FILE *log_file = fopen("access.log", "a");
    if (log_file == NULL)
    {
        perror("fopen");
        return;
    }
    fprintf(
        log_file,
        "%s %s %s %d %zu\n",
        log->ip,
        log->method,
        log->path,
        log->status_code,
        log->bytes_sent
    );
    fclose(log_file);
    return;

}