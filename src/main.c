#include "server/server.h"
#include <stdio.h>
#define PORT 8080
int main(void)
{
    int port = PORT;
    int server_fd = start_server(port);
    if (server_fd < 0)
    {
        printf("Failed to create\n");
        return 1;
    }
    printf("Created successfully\n");

    return 0;
}