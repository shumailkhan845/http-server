#ifndef HANDLER_H
#define HANDLER_H

#include "http/request.h"
#include "http/response.h"
void handle_login(struct http_request *request, struct http_response *response);

void handle_echo(struct http_request *request, struct http_response *response);




#endif