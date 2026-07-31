#include "server.h"
#include "http/request.h"
#include "http/response.h"
#include "router/router.h"
#include "file/file.h"
#include "mime/mime.h"

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
        printf("Client IP address: %s\n", ip);
        printf("Client Port: %d\n", ntohs(client_addr.sin_port));
        printf("Client Connected\n");

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
        // printf("Recived : %s\n", buffer);

        // Preparing the http request
        struct http_request request = parse_http_request(buffer);

        printf("Method : %s\n", request.method);
        printf("Path : %s\n", request.path);
        printf("Request : %s\n", request.version);
        
        /* Preparing the http response */
        struct http_response response;
        const char *filepath = route_request(request.path);
        if (filepath == NULL)
        {
            filepath = "public/404.html";
            response = create_http_response(404);
        }
        else
        {
            response = create_http_response(200);
        }
        printf("File path : %s\n", filepath);
        char *body = read_file(filepath);
        response.body = body;
        response.content_type = get_mime_type(filepath);;
        /* Serializing the http response */
        char *data = serilize_http_response(&response);

        // printf("Data : %s", data);

        int s = send(new_fd, data, strlen(data), 0);
        if (s < 0)
        {
            perror("send");
            return -1;
        }
        free(data);
        free(body);
        free(filepath);
        close(new_fd);
    }
    close(sockfd);
    return -1;
}