#include "router.h"
#include "handler/handler.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define PUBLIC_DIR "public"
struct route route_request(struct http_request *request)
{
    struct route route = {0};
    char *path = NULL;

    path = request->path;

    /* HANDLING THE DYNAMIC ROUTES */
    if (strcmp(path, "/login") == 0)
    {
        route.filepath = NULL;
        route.type = ROUTE_DYNAMIC;
        route.handler = handle_login;

        return route;
    }
    if (strcmp(path, "/echo") == 0)
    {
        route.filepath = NULL;
        route.type = ROUTE_DYNAMIC;
        route.handler = handle_echo;

        return route;
    }
    /* HANDLING THE STATIC ROUTES */


    if (strcmp(path, "/") == 0)
    {
        path = "/index.html";
    }

    size_t size = strlen(PUBLIC_DIR) + strlen(path) + 1;

    char *file = malloc(size);
    if (file == NULL)
    {
        return route;
    }

    strcpy(file, PUBLIC_DIR);
    strcat(file, path);

    route.filepath = file;
    route.type = ROUTE_STATIC;
    route.handler = NULL;

    return route;
}