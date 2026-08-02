#ifndef LOGGER_H
#define LOGGER_H
// #include <netinet/in.h>
#include <arpa/inet.h>

#include <stdio.h>
struct log_entry
{
    char ip[INET_ADDRSTRLEN];
    char *method;
    char *path;
    int status_code;
    size_t bytes_sent;
};
void log_access(struct log_entry *logs);

#endif