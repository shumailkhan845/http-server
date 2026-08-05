#include <stdio.h>
void handle_login(struct http_request *request, struct http_response *response)
{
    printf("Hey Server it is login\n");
    return;
}

void handle_echo(struct http_request *request, struct http_response *response)
{
    printf("Hey Server it is echo\n");
    return;
}