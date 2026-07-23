#include "router.h"
#include <string.h>
const char *route_request(const char *path)
{
    if(strcmp(path, "/") == 0)
    {
        return "public/index.html";
    }
    if(strcmp(path, "/about") == 0)
    {
        return "public/about.html";
    }
    return NULL;
}