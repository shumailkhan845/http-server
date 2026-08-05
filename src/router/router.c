#include "router.h"
#include "handler/handler.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define PUBLIC_DIR "public"
struct route route_request(struct http_request *request)
{
    struct route route = {0};
    char *filepath = NULL;

    filepath = request->path;

    /* HANDLING THE DYNAMIC ROUTES */
    if (strcmp(filepath, "/login") == 0)
    {
        route.filepath = NULL;
        route.type = ROUTE_DYNAMIC;
        route.handler = handle_login;

        return route;
    }
    if (strcmp(filepath, "/echo") == 0)
    {
        route.filepath = NULL;
        route.type = ROUTE_DYNAMIC;
        route.handler = handle_echo;
        return route;
    }
    /* HANDLING THE STATIC ROUTES */

    size_t size = (strlen(PUBLIC_DIR) + strlen(filepath) + 1);
    printf("Size = %zu\n", size);
    char *file = malloc(size);

    if (file == NULL)
    {
        return route;
    }

    if (strcmp(filepath, "/") == 0)
    {
        strcpy(file, PUBLIC_DIR);
        filepath = "/index.html";
        strcat(file, filepath);
        route.filepath = file;
        route.type = ROUTE_STATIC;
        route.handler = NULL;

        return route;
    }
    else
    {

        strcpy(file, PUBLIC_DIR);
        strcat(file, filepath);
        route.filepath = file;
        route.type = ROUTE_STATIC;
        route.handler = NULL;
        return route;
    }

    return route;
}