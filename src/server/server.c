#include "server.h"

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
    buffer[recv_result - 1] = '\0';
    printf("Recv_result %d\n", recv_result);
    printf("Recived : %s\n", buffer);

    // Implementing the send()
    char *msg = "Hello from server";
    int len = strlen(msg);
    int send_result = send(new_fd, msg, len, 0);
    if (send_result < 0)
    {
        perror("send");
        close(new_fd);
        close(sockfd);
        return -1;
    }

    return new_fd;
}