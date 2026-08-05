#include "server.h"
#include "http/request.h"
#include "http/response.h"
#include "router/router.h"
#include "file/file.h"
#include "mime/mime.h"
#include "logger/logger.h"

#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>

#define BACKLOG 5

int start_server(int port)
{

    char ip[INET_ADDRSTRLEN];

    // Create the server socket
    int new_fd = 0;
    socklen_t addr_len;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("socket");
        return -1;
    }
    printf("Server socket created successfully\n");

    // Initializing the addresses
    struct sockaddr_in server_addr, client_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    memset(&client_addr, 0, sizeof(client_addr));

    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    // Binding the sockets
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind");
        close(sockfd);
        return -1;
    }

    // Listening on the port
    if (listen(sockfd, BACKLOG) < 0)
    {
        perror("listen");
        close(sockfd);
        return -1;
    }
    while (1)
    {
        addr_len = sizeof(client_addr);
        new_fd = accept(sockfd, (struct sockaddr *)&client_addr, &addr_len);
        if (new_fd < 0)
        {
            perror("accept");
            close(sockfd);
            return -1;
        }
        inet_ntop(AF_INET, &client_addr.sin_addr, ip, sizeof(ip));
        // printf("Client IP address: %s\n", ip);
        // printf("Client Port: %d\n", ntohs(client_addr.sin_port));
        // printf("Client Connected\n");

        // Impelmenting the recv
        char buffer[1024];
        int recv_result = recv(new_fd, buffer, sizeof(buffer), 0);
        if (recv_result < 0)
        {
            perror("recv");
            close(new_fd);
            close(sockfd);
            return -1;
        }
        else if (recv_result == 0)
        {
            printf("Connection closed\n");
            close(new_fd);
            close(sockfd);
            return -1;
        }
        buffer[recv_result] = '\0';
        // printf("Recv_result %d\n", recv_result);
        printf("Recived : %s\n", buffer);

        /* PREPARING THE HTTP REQUEST  */
        struct http_request request = parse_http_request(buffer);

        printf("Method : %s\n", request.method);
        printf("Path : %s\n", request.path);
        printf("Request : %s\n", request.version);

        /* IF HTTP REQUEST FAILED */
        if (request.method == NULL || request.path == NULL || request.version == NULL)
        {
            continue;
            // Return 400 Bad Request
        }

        /* PREPARING THE HTTP RESPONSE  */
        struct http_response response;

        /* DEFINING THE ROUTER STRUCT FROM ROUTER.H AND HANDLING THE PATH REDIRECTION */

        struct route router;
        router = route_request(&request);
        if (router.type == ROUTE_STATIC)
        {
            printf("Inside static route \n\n");
            if (router.filepath == NULL)
            {
                router.filepath = "public/404.html";
                response = create_http_response(404);
            }
            else
            {
                printf("Inside static route else block\n\n");

                struct file_data file = {0};

                file = read_file(router.filepath);
                response = create_http_response(200);
                response.body = file.data;

                response.content_type = get_mime_type(router.filepath);

                response.content_length = file.size;
            }
        }
        else
        {
            printf("Calling dynamic handler...\n");
            router.handler(&request, &response);
        }

        /* SERIALIZING THE HTTP HEADER */
        char *header = serilize_http_header(&response);

        // printf("Data : %s", data);

        int s_headers = send(new_fd, header, strlen(header), 0);
        if (s_headers < 0)
        {
            perror("send");
            return -1;
        }
        /* Serializing the http body */

        int s_body = send(new_fd, response.body, response.content_length, 0);
        // printf("Headers : %s\n", header);
        // printf("Response Body : %ld\n", response.body);

        /* Generating the server log */
        struct log_entry log;
        strncpy(log.ip, ip, INET_ADDRSTRLEN);
        log.ip[INET_ADDRSTRLEN - 1] = '\0';
        log.method = request.method;
        log.path = request.path;
        log.status_code = response.status_code;
        log.bytes_sent = response.content_length;
        log_access(&log);

        // Freeing and closing the blah blah blah...!!!!!
        free(header);
        free(router.filepath);
        close(new_fd);
    }
    close(sockfd);
    return -1;
}