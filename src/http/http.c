// #include "http.h"
// #include <string.h>
// #define DELIMITER " \t\r\n\a"
// struct http_request request;
// struct http_request parse_http_request(char *buffer)
// {
    
//     char *token;
//     token = strtok(buffer, DELIMITER);
//     request.method = token;
//     token = strtok(NULL, DELIMITER);
//     request.path = token;
//     token = strtok(NULL, DELIMITER);
//     request.version = token;

//     return request;
// }