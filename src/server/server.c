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
#include <pthread.h>

#include "server.h"
#include "http/request.h"
#include "router/router.h"
#include "http/response.h"
#include "file/file.h"
#include "handler/handler.h"
#include "logger/logger.h"
#include "error/error.h"
#include "mime/mime.h"

#define BACKLOG 5
#define BUFFER_SIZE 1024


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

        /*
         * We have received the complete HTTP header.
         */
        if (strstr(buffer, "\r\n\r\n") != NULL)
        {
            return offset;
        }
    }

    /*
     * Request did not fit in the buffer.
     */
    return -1;
}


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


/*
 * Send an HTTP error response.
 *
 * Error response bodies are dynamically allocated,
 * therefore the caller must free response.body.
 */
static int send_error_response(int fd, int status_code)
{
    struct http_response response =
        build_error_response(status_code);

    char *header =
        serilize_http_header(&response);

    if (header == NULL)
    {
        free(response.body);
        return -1;
    }

    int header_length =
        strlen(header);

    if (send_all(
            fd,
            header,
            header_length
        ) < 0)
    {
        free(header);
        free(response.body);
        return -1;
    }

    free(header);

    if (response.body != NULL &&
        response.content_length > 0)
    {
        if (send_all(
                fd,
                response.body,
                response.content_length
            ) < 0)
        {
            free(response.body);
            return -1;
        }
    }

    free(response.body);

    return 0;
}


void *handle_client(void *arg)
{
    /*
     * Get client's socket.
     */
    int new_fd = *(int *)arg;

    /*
     * Thread now owns this allocation.
     */
    free(arg);


    /*
     * Receive HTTP request.
     */
    char buffer[BUFFER_SIZE];

    int recv_result =
        recv_all(
            new_fd,
            buffer,
            sizeof(buffer)
        );

    if (recv_result < 0)
    {
        close(new_fd);
        return NULL;
    }

    if (recv_result == 0)
    {
        close(new_fd);
        return NULL;
    }


    /*
     * Parse HTTP request.
     */
    struct http_request request =
        parse_http_request(buffer);

    if (request.method == NULL ||
        request.path == NULL ||
        request.version == NULL)
    {
        send_error_response(
            new_fd,
            400
        );

        close(new_fd);

        return NULL;
    }

    printf(
        "Request: %s %s %s\n",
        request.method,
        request.path,
        request.version
    );


    /*
     * Find Content-Length.
     */
    int content_length = 0;

    for (int i = 0;
         i < request.header_count;
         i++)
    {
        if (strcasecmp(
                request.headers[i].key,
                "Content-Length"
            ) == 0)
        {
            char *endptr;

            long value =
                strtol(
                    request.headers[i].value,
                    &endptr,
                    10
                );

            /*
             * Invalid Content-Length.
             */
            if (endptr ==
                    request.headers[i].value ||
                *endptr != '\0' ||
                value < 0 ||
                value > INT_MAX)
            {
                send_error_response(
                    new_fd,
                    400
                );

                close(new_fd);

                return NULL;
            }

            content_length =
                (int)value;

            break;
        }
    }


    /*
     * Receive remaining request body.
     */
    if (request.body != NULL)
    {
        int body_offset =
            (int)(request.body - buffer);

        int body_received =
            recv_result - body_offset;

        if (body_received < 0)
        {
            close(new_fd);
            return NULL;
        }

        int remaining_body =
            content_length -
            body_received;


        /*
         * More body data was received
         * than Content-Length specifies.
         */
        if (remaining_body < 0)
        {
            send_error_response(
                new_fd,
                400
            );

            close(new_fd);

            return NULL;
        }


        /*
         * Make sure remaining body
         * fits in our buffer.
         */
        int available_space =
            sizeof(buffer) -
            body_offset -
            1;

        if (remaining_body >
            available_space)
        {
            send_error_response(
                new_fd,
                400
            );

            close(new_fd);

            return NULL;
        }


        /*
         * Receive remaining body.
         */
        while (remaining_body > 0)
        {
            int bytes_received =
                recv(
                    new_fd,
                    buffer + body_offset,
                    remaining_body,
                    0
                );

            if (bytes_received < 0)
            {
                perror("recv");

                close(new_fd);

                return NULL;
            }

            if (bytes_received == 0)
            {
                close(new_fd);

                return NULL;
            }

            body_offset +=
                bytes_received;

            remaining_body -=
                bytes_received;

            buffer[body_offset] =
                '\0';
        }

        /*
         * Body must be complete.
         */
        if (remaining_body != 0)
        {
            close(new_fd);

            return NULL;
        }
    }


    /*
     * Route request.
     */
    struct route router =
        route_request(&request);

    struct http_response response = {0};

    /*
     * Tracks whether response.body
     * was dynamically allocated.
     *
     * 1 = safe to free
     * 0 = do not free
     */
    int body_owned = 0;


    /*
     * Static route.
     */
    if (router.type == ROUTE_STATIC)
    {
        if (router.filepath == NULL)
        {
            response =
                build_error_response(404);

            body_owned = 1;
        }
        else
        {
            struct file_data file =
                read_file(
                    router.filepath
                );

            if (file.status == 404)
            {
                response =
                    build_error_response(404);

                body_owned = 1;
            }
            else if (file.status != 200)
            {
                response =
                    build_error_response(500);

                body_owned = 1;
            }
            else
            {
                response =
                    create_http_response(200);

                response.body =
                    file.data;

                response.content_type =
                    get_mime_type(
                        router.filepath
                    );

                response.content_length =
                    file.size;

                /*
                 * file.data came from malloc().
                 */
                body_owned = 1;
            }
        }
    }


    /*
     * Dynamic route.
     */
    else if (router.type == ROUTE_DYNAMIC)
    {
        if (router.handler != NULL)
        {
            response =
                create_http_response(200);

            router.handler(
                &request,
                &response
            );

            /*
             * Dynamic handlers currently use
             * string literals for their bodies.
             */
            body_owned = 0;
        }
        else
        {
            response =
                build_error_response(500);

            body_owned = 1;
        }
    }


    /*
     * Unknown route.
     */
    else
    {
        response =
            build_error_response(404);

        body_owned = 1;
    }


    /*
     * Serialize HTTP response.
     */
    char *header =
        serilize_http_header(
            &response
        );

    if (header == NULL)
    {
        free(router.filepath);

        if (body_owned)
        {
            free(response.body);
        }

        close(new_fd);

        return NULL;
    }


    /*
     * Send response header.
     */
    int header_length =
        strlen(header);

    if (send_all(
            new_fd,
            header,
            header_length
        ) < 0)
    {
        free(header);
        free(router.filepath);

        if (body_owned)
        {
            free(response.body);
        }

        close(new_fd);

        return NULL;
    }

    free(header);


    /*
     * Send response body.
     */
    if (response.body != NULL &&
        response.content_length > 0)
    {
        if (send_all(
                new_fd,
                response.body,
                response.content_length
            ) < 0)
        {
            free(router.filepath);

            if (body_owned)
            {
                free(response.body);
            }

            close(new_fd);

            return NULL;
        }
    }


    /*
     * Cleanup.
     */
    free(router.filepath);

    if (body_owned)
    {
        free(response.body);
    }

    close(new_fd);

    return NULL;
}


int start_server(int port)
{
    int sockfd;

    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;


    /*
     * Create socket.
     */
    sockfd =
        socket(
            AF_INET,
            SOCK_STREAM,
            0
        );

    if (sockfd < 0)
    {
        perror("socket");

        return -1;
    }


    /*
     * Allow immediate reuse of port.
     */
    int opt = 1;

    if (setsockopt(
            sockfd,
            SOL_SOCKET,
            SO_REUSEADDR,
            &opt,
            sizeof(opt)
        ) < 0)
    {
        perror("setsockopt");

        close(sockfd);

        return -1;
    }


    /*
     * Configure server address.
     */
    memset(
        &server_addr,
        0,
        sizeof(server_addr)
    );

    server_addr.sin_family =
        AF_INET;

    server_addr.sin_addr.s_addr =
        htonl(INADDR_ANY);

    server_addr.sin_port =
        htons(port);


    /*
     * Bind.
     */
    if (bind(
            sockfd,
            (struct sockaddr *)&server_addr,
            sizeof(server_addr)
        ) < 0)
    {
        perror("bind");

        close(sockfd);

        return -1;
    }


    /*
     * Listen.
     */
    if (listen(
            sockfd,
            BACKLOG
        ) < 0)
    {
        perror("listen");

        close(sockfd);

        return -1;
    }

    printf(
        "Server listening on port %d...\n",
        port
    );


    /*
     * Accept clients forever.
     */
    while (1)
    {
        socklen_t addr_len =
            sizeof(client_addr);

        int new_fd =
            accept(
                sockfd,
                (struct sockaddr *)&client_addr,
                &addr_len
            );

        if (new_fd < 0)
        {
            perror("accept");
            continue;
        }


        /*
         * Allocate storage for this
         * client's socket fd.
         */
        int *thread_fd =
            malloc(sizeof(int));

        if (thread_fd == NULL)
        {
            perror("malloc");

            close(new_fd);

            continue;
        }


        /*
         * Copy fd into thread-owned memory.
         */
        *thread_fd = new_fd;


        /*
         * Create client thread.
         */
        pthread_t thread;

        int result =
            pthread_create(
                &thread,
                NULL,
                handle_client,
                thread_fd
            );


        /*
         * Thread creation failed.
         */
        if (result != 0)
        {
            fprintf(
                stderr,
                "pthread_create failed: %s\n",
                strerror(result)
            );

            free(thread_fd);

            close(new_fd);

            continue;
        }


        /*
         * No pthread_join() needed.
         * The thread cleans itself up.
         */
        pthread_detach(thread);
    }


    close(sockfd);

    return 0;
}