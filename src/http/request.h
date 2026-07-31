#ifndef REQUEST_H
#define REQUEST_H

#define MAX_SIZE 32
//Defining the map
struct http_headers 
{
    char *key;
    char *value;
};
// Defining the http request struct
struct http_request
{
    char *method;
    char *path;
    char *version;

    struct http_headers headers[MAX_SIZE];
    int header_count;
};


struct http_request parse_http_request(char *buffer);

#endif