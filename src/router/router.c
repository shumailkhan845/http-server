#include "router.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

char *route_request(const char *path)
{
    char *filepath = NULL;

    if (strcmp(path, "/") == 0)
    {
        filepath = malloc(strlen("public/index.html") + 1);
        if (filepath == NULL)
        {
            return NULL;
        }

        strcpy(filepath, "public/index.html");
        return filepath;
    }

    size_t len = strlen("public") + strlen(path) + 1;

    filepath = malloc(len);
    if (filepath == NULL)
    {
        return NULL;
    }

    strcpy(filepath, "public");
    strcat(filepath, path);

    return filepath;
}