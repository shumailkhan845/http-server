#include "response.h"

#include <stdio.h>
#include <stdlib.h>

struct http_response create_http_response(int status)
{
    struct http_response response = {0};

    response.version = "HTTP/1.1";
    response.status_code = status;

    return response;
}

char *serilize_http_header(struct http_response *response)
{
    switch (response->status_code)
    {
    case 200:
        response->reason_phrase = "OK";
        break;

    case 400:
        response->reason_phrase = "Bad Request";
        break;

    case 401:
        response->reason_phrase = "Unauthorized";
        break;

    case 404:
        response->reason_phrase = "Page not found";
        break;

    case 405:
        response->reason_phrase = "Method Not Allowed";
        break;

    case 500:
        response->reason_phrase = "Internal Server Error";
        break;

    default:
        response->reason_phrase = "Unknown";
        break;
    }

    char *header = malloc(1024);

    if (header == NULL)
    {
        perror("malloc");
        return NULL;
    }

    snprintf(
        header,
        1024,
        "%s %d %s\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %zu\r\n"
        "Connection: close\r\n"
        "\r\n",
        response->version,
        response->status_code,
        response->reason_phrase,
        response->content_type,
        response->content_length
    );

    return header;
}