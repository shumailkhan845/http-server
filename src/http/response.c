#include "response.h"

#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
struct http_response create_http_response(int status)
{
    struct http_response response;

    response.status_code = status;
    return response;
}

char *serilize_http_header(struct http_response *response)
{
    if (response->status_code == 200)
    {
        response->reason_phrase = "OK";
    }

    if(response->status_code == 401)
    {
        response->reason_phrase = "Unauthorized";
    }

    if(response->status_code == 400)
    {
        response->reason_phrase = "Bad Request";
    }

    if(response->status_code == 500)
    {
        response->reason_phrase = "Internal Server Error";
    }

    if (response->status_code == 404)
    {
        response->reason_phrase = "Page not found";
    }

    char *header = malloc(1024);
    if (header == NULL)
    {
        perror("malloc");
        return NULL;
    }

    sprintf(
        header,
        "HTTP/1.1 %d %s\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %zu\r\n"
        "\r\n",
        response->status_code,
        response->reason_phrase,
        response->content_type,
        response->content_length);

    return header;
}


