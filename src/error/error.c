#include "error.h"
#include "http/response.h"
#include "file/file.h"
#include "mime/mime.h"
struct http_response build_error_response(int status_code)
{
    struct http_response response = {0};
    const char *filepath = NULL;

    switch (status_code)
    {
    case 400:
        filepath = "public/errors/400.html";
        break;
    case 404:
        filepath = "public/errors/404.html";
        break;
    case 405:
        filepath = "public/errors/405.html";
        break;
    case 500:
        filepath = "public/errors/500.html";
        break;
    default:
        return response;
    }

                    /* FILL RESPONSE */

    struct file_data f_data = read_file(filepath);
    response.body = f_data.data;
    response.content_length = f_data.size;
    response.status_code = status_code;
    response.content_type = get_mime_type(filepath);

    return response;
}