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

char *serilize_http_response(struct http_response *response)
{
    if (response->status_code == 200)
    {
        response->reason_phrase = "OK";
    }
    if (response->status_code == 404)
    {
        response->reason_phrase = "Page not found";
    }
    char *data = malloc(1024);

    sprintf(
    data,
    "HTTP/1.1 %d %s\r\n"
    "Content-Type: %s\r\n"
    "Content-Length: %ld\r\n"
    "\r\n"
    "%s",
    response->status_code,
    response->reason_phrase,
    response->content_type,
    strlen(response->body),
    response->body
);
    return data;
}