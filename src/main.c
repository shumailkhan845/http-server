#include "server/server.h"
#include <stdio.h>
int main(void)
{
    int port = 8080;
    int server_fd = start_server(port);
    if (server_fd < 0)
    {
        printf("Failed to create\n");
        return 1;
    }
    printf("Created successfully\n");

    return 0;
}