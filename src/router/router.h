#ifndef ROUTER_H
#define ROUTER_H

#include "http/request.h"
#include "http/response.h"
typedef void (*route_handler)(struct http_request *, struct http_response *);

enum route_type
{
    ROUTE_STATIC,
    ROUTE_DYNAMIC
};
struct route
{
    enum route_type type;
    char *filepath;
    route_handler handler;

};
struct route route_request(struct http_request *request);

#endif