#ifndef RESPONSE_H
#define RESPONSE_H
#include <stddef.h>

// Defining the http response struct
struct http_response
{
    char *version;
    int status_code;
    char *reason_phrase;

    char *content_type;
    size_t content_length;

    char *body;
};
struct http_response create_http_response(int status);
char *serilize_http_response(struct http_response *response);


#endif