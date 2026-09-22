#include "server.h"
#include "http/request.h"
#include "http/response.h"
#include "router/router.h"
#include "file/file.h"
#include "mime/mime.h"
#include "logger/logger.h"
#include "error/error.h"

#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <errno.h>
#include <limits.h>

#define BACKLOG 5

int start_server(int port)
{
    char ip[INET_ADDRSTRLEN];

    /* Create server socket */
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
    {
        perror("socket");
        return -1;
    }

    printf("Server socket created successfully\n");

    /* Initialize addresses */
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;

    memset(&server_addr, 0, sizeof(server_addr));
    memset(&client_addr, 0, sizeof(client_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    /* Bind */
    if (bind(sockfd,
             (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0)
    {
        perror("bind");
        close(sockfd);
        return -1;
    }

    /* Listen */
    if (listen(sockfd, BACKLOG) < 0)
    {
        perror("listen");
        close(sockfd);
        return -1;
    }

    while (1)
    {
        socklen_t addr_len = sizeof(client_addr);

        int new_fd = accept(
            sockfd,
            (struct sockaddr *)&client_addr,
            &addr_len
        );

        if (new_fd < 0)
        {
            perror("accept");
            continue;
        }

        if (inet_ntop(
                AF_INET,
                &client_addr.sin_addr,
                ip,
                sizeof(ip)) == NULL)
        {
            perror("inet_ntop");
            close(new_fd);
            continue;
        }

        /* =========================================================
         * RECEIVE REQUEST
         * ========================================================= */

        char buffer[1024];

        int recv_result = recv_all(
            new_fd,
            buffer,
            sizeof(buffer)
        );

        if (recv_result < 0)
        {
            close(new_fd);
            continue;
        }

        if (recv_result == 0)
        {
            printf("Connection closed\n");
            close(new_fd);
            continue;
        }

        /* Find beginning of body */

        char *body_start = strstr(buffer, "\r\n\r\n");

        if (body_start == NULL)
        {
            close(new_fd);
            continue;
        }

        int body_offset = (int)((body_start + 4) - buffer);

        int body_received = recv_result - body_offset;

        buffer[recv_result] = '\0';

        printf("Received:\n%s\n", buffer);

        /* =========================================================
         * PARSE REQUEST
         * ========================================================= */

        struct http_request request = parse_http_request(buffer);

        /* Validate request line */

        if (request.method == NULL ||
            request.path == NULL ||
            request.version == NULL)
        {
            struct http_response response = build_error_response(400);

            char *header = serilize_http_header(&response);

            if (header != NULL)
            {
                send_all(new_fd, header, strlen(header));
                send_all(
                    new_fd,
                    response.body,
                    response.content_length
                );

                free(header);
            }

            close(new_fd);
            continue;
        }

        /* =========================================================
         * FIND CONTENT-LENGTH
         * ========================================================= */

        int content_length = 0;
        int content_length_found = 0;
        int invalid_content_length = 0;

        for (int i = 0; i < request.header_count; i++)
        {
            if (strcasecmp(
                    request.headers[i].key,
                    "Content-Length") == 0)
            {
                char *endptr;

                errno = 0;

                long value = strtol(
                    request.headers[i].value,
                    &endptr,
                    10
                );

                if (errno != 0 ||
                    endptr == request.headers[i].value ||
                    *endptr != '\0' ||
                    value < 0 ||
                    value > INT_MAX)
                {
                    invalid_content_length = 1;
                    break;
                }

                content_length = (int)value;
                content_length_found = 1;

                break;
            }
        }

        if (invalid_content_length)
        {
            struct http_response response = build_error_response(400);

            char *header = serilize_http_header(&response);

            if (header != NULL)
            {
                send_all(new_fd, header, strlen(header));
                send_all(
                    new_fd,
                    response.body,
                    response.content_length
                );

                free(header);
            }

            close(new_fd);
            continue;
        }

        /*
         * If Content-Length doesn't exist,
         * assume there is no request body.
         */
        if (!content_length_found)
        {
            content_length = 0;
        }

        /* =========================================================
         * BODY VALIDATION
         * ========================================================= */

        /*
         * body_received represents body bytes that arrived
         * together with the headers.
         */

        if (body_received > content_length)
        {
            printf("Received more body data than Content-Length\n");

            close(new_fd);
            continue;
        }

        /*
         * Check whether the entire body can fit into our buffer.
         *
         * We reserve one byte for '\0'.
         */

        int available_body_space =
            (int)sizeof(buffer) - body_offset - 1;

        if (content_length > available_body_space)
        {
            printf("Request body too large\n");

            struct http_response response =
                build_error_response(400);

            char *header =
                serilize_http_header(&response);

            if (header != NULL)
            {
                send_all(
                    new_fd,
                    header,
                    strlen(header)
                );

                send_all(
                    new_fd,
                    response.body,
                    response.content_length
                );

                free(header);
            }

            close(new_fd);
            continue;
        }

        /* =========================================================
         * RECEIVE REMAINING BODY
         * ========================================================= */

        int remaining_body =
            content_length - body_received;

        while (remaining_body > 0)
        {
            int available_space =
                (int)sizeof(buffer) - body_offset - 1;

            if (available_space <= 0)
            {
                printf("No buffer space remaining\n");
                break;
            }

            int bytes_to_receive = remaining_body;

            if (bytes_to_receive > available_space)
            {
                bytes_to_receive = available_space;
            }

            int bytes_received = recv(
                new_fd,
                buffer + body_offset,
                bytes_to_receive,
                0
            );

            if (bytes_received < 0)
            {
                perror("recv");
                break;
            }

            if (bytes_received == 0)
            {
                printf("Client closed connection before body completed\n");
                break;
            }

            body_offset += bytes_received;
            remaining_body -= bytes_received;

            buffer[body_offset] = '\0';
        }

        /*
         * Body wasn't completely received.
         */
        if (remaining_body != 0)
        {
            printf("Incomplete request body\n");

            close(new_fd);
            continue;
        }

        printf("Content received successfully\n");

        /* =========================================================
         * REQUEST INFORMATION
         * ========================================================= */

        printf("Method  : %s\n", request.method);
        printf("Path    : %s\n", request.path);
        printf("Version : %s\n", request.version);

        /* =========================================================
         * PREPARE RESPONSE
         * ========================================================= */

        struct http_response response = {0};

        /* =========================================================
         * ROUTING
         * ========================================================= */

        struct route router = route_request(&request);

        /* =========================================================
         * STATIC ROUTE
         * ========================================================= */

        if (router.type == ROUTE_STATIC)
        {
            printf("Inside static route...\n\n");

            if (router.filepath == NULL)
            {
                response = build_error_response(404);
            }
            else
            {
                struct file_data file = {0};

                file = read_file(router.filepath);

                if (file.status == 404)
                {
                    response = build_error_response(404);
                }
                else if (file.status != 200)
                {
                    response = build_error_response(500);
                }
                else
                {
                    response = create_http_response(200);

                    response.body = file.data;
                    response.content_type =
                        get_mime_type(router.filepath);
                    response.content_length =
                        file.size;
                }
            }
        }

        /* =========================================================
         * DYNAMIC ROUTE
         * ========================================================= */

        else
        {
            printf("Calling dynamic handler...\n");

            if (router.handler != NULL)
            {
                router.handler(
                    &request,
                    &response
                );
            }
            else
            {
                response =
                    build_error_response(404);
            }
        }

        /* =========================================================
         * SERIALIZE HEADER
         * ========================================================= */

        char *header =
            serilize_http_header(&response);

        if (header == NULL)
        {
            free(router.filepath);
            close(new_fd);
            continue;
        }

        /* =========================================================
         * SEND HEADER
         * ========================================================= */

        if (send_all(
                new_fd,
                header,
                strlen(header)) < 0)
        {
            printf("Header sending failed\n");

            free(header);
            free(router.filepath);
            close(new_fd);

            continue;
        }

        /* =========================================================
         * SEND BODY
         * ========================================================= */

        if (response.body != NULL &&
            response.content_length > 0)
        {
            if (send_all(
                    new_fd,
                    response.body,
                    response.content_length) < 0)
            {
                printf("Body sending failed\n");

                free(header);
                free(router.filepath);
                close(new_fd);

                continue;
            }
        }

        /* =========================================================
         * LOG REQUEST
         * ========================================================= */

        struct log_entry log;

        strncpy(
            log.ip,
            ip,
            INET_ADDRSTRLEN
        );

        log.ip[INET_ADDRSTRLEN - 1] = '\0';

        log.method = request.method;
        log.path = request.path;
        log.status_code = response.status_code;
        log.bytes_sent = response.content_length;

        log_access(&log);

        /* =========================================================
         * CLEANUP
         * ========================================================= */

        free(header);
        free(router.filepath);

        close(new_fd);
    }

    close(sockfd);

    return 0;
}


/* =============================================================
 * SEND ALL
 * ============================================================= */

int send_all(int fd, char *buffer, int size)
{
    int offset = 0;

    while (offset < size)
    {
        int bytes_sent = send(
            fd,
            buffer + offset,
            size - offset,
            0
        );

        if (bytes_sent < 0)
        {
            perror("send");
            return -1;
        }

        if (bytes_sent == 0)
        {
            return -1;
        }

        offset += bytes_sent;
    }

    return 0;
}


/* =============================================================
 * RECEIVE UNTIL HTTP HEADERS ARE COMPLETE
 * ============================================================= */

int recv_all(int fd, char *buffer, int size)
{
    int offset = 0;

    while (offset < size - 1)
    {
        int bytes_recv = recv(
            fd,
            buffer + offset,
            size - 1 - offset,
            0
        );

        if (bytes_recv < 0)
        {
            perror("recv");
            return -1;
        }

        if (bytes_recv == 0)
        {
            return 0;
        }

        offset += bytes_recv;

        buffer[offset] = '\0';

        if (strstr(buffer, "\r\n\r\n") != NULL)
        {
            return offset;
        }
    }

    /*
     * Header section didn't fit in the buffer.
     */
    return -1;
}