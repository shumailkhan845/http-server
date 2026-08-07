#include <stdio.h>
#include <string.h>
#include "handler/handler.h"

#include "form/form.h"

void handle_login(struct http_request *request, struct http_response *response)
{
    printf("Hey Server it is login\n");
    struct form_data data = parse_form(request->body);
    char *username = NULL;
    char *password = NULL;
    size_t i = 0;
    while (i < data.field_count)
    {
        if (data.fields[i].key != NULL)
        {
            i++;
            continue;
        }

        if (strcmp(data.fields[i].key, "username") == 0)
        {
            username = data.fields[i].value;
        }
        if (strcmp(data.fields[i].key, "password") == 0)
        {
            password = data.fields[i].value;
        }
        i++;
    }
    if (username == NULL || password == NULL)
    {
        response->body = "400 Bad Request";
        size_t len = strlen(response->body);
        response->content_length = len;
        response->content_type = "text/plain";
        response->status_code = 400;
        return;
    }

    if (strcmp(username, "admin") == 0 && strcmp(password, "123") == 0)
    {
        response->body = "Login Successful";
        size_t len = strlen(response->body);
        response->content_length = len;
        response->content_type = "text/plain";
        response->status_code = 200;
        return;
    }

    else
    {
        response->body = "Invalid username or password";
        size_t len = strlen(response->body);
        response->content_length = len;
        response->content_type = "text/plain";
        response->status_code = 401;
        return;
    }
}

void handle_echo(struct http_request *request, struct http_response *response)
{
    printf("Hey Server it is echo\n");
    return;
}