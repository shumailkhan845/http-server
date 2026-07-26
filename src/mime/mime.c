#include "mime.h"
#include <string.h>
const char *get_mime_type(const char *filepath)
{
    char *mime = strrchr(filepath, '.');

    if (strcmp(".html", mime) == 0)
    {
        return "text/html";
    }
    else if (strcmp(".css", mime) == 0)
    {
        return "text/css";
    }
    else if (strcmp(".js", mime) == 0)
    {
        return "application/javascript";
    }
    else if (strcmp(".png", mime) == 0)
    {
        return "image/png";
    }
    else if (strcmp(".jpg", mime) == 0)
    {
        return "image/jpeg";
    }
    else if (strcmp(".jpeg", mime) == 0)
    {
        return "image/jpeg";
    }
    else if (strcmp(".ico", mime) == 0)
    {
        return "image/x-icon";
    }
    else if (strcmp(".txt", mime) == 0)
    {
        return "text/plain";
    }
    else
    {
        return "application/octet-stream";
    }
}