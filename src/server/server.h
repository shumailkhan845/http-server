#ifndef SERVER_H
#define SERVER_H
#include <netinet/in.h>

int start_server(int port);

int send_all(int fd, char *buffer, int size);

int recv_all(int fd, char *buffer, int size);




#endif