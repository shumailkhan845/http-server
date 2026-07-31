#include "request.h"
#include <string.h>
#include <stdio.h>

#define DELIMITER "\r\n"
#define MAX_HEADERS 32

struct http_request parse_http_request(char *buffer)
{
    struct http_request request = {0};

    /* Parse request line */
    char *line = strtok(buffer, DELIMITER);

    if (line == NULL)
    {
        return request;
    }

    char *space1 = strchr(line, ' ');
    if (space1 == NULL)
    {
        return request;
    }

    *space1 = '\0';
    request.method = line;

    char *path = space1 + 1;

    char *space2 = strchr(path, ' ');
    if (space2 == NULL)
    {
        return request;
    }

    *space2 = '\0';
    request.path = path;

    request.version = space2 + 1;

    /* Parse headers */
    while ((line = strtok(NULL, DELIMITER)) != NULL)
    {
        if (*line == '\0')
        {
            break;
        }

        char *colon = strchr(line, ':');

        if (colon == NULL)
        {
            continue;
        }

        *colon = '\0';

        request.headers[request.header_count].key = line;
        request.headers[request.header_count].value = colon + 1;

        while (*request.headers[request.header_count].value == ' ')
        {
            request.headers[request.header_count].value++;
        }

        request.header_count++;

        if (request.header_count >= MAX_HEADERS)
        {
            break;
        }
    }

    return request;
}