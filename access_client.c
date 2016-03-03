/*============================================================================
 * Name        : access_client.c
 * Author      : fallenworld
 * Version     : 0.1
 * Copyright   :
 * Description : The access client
 * ============================================================================*/
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SOCKET_RECV_BUF_SIZE 512

#define DISCONNECT(fd, log) \
    do \
    { \
        puts( (log) ); \
        close( (fd) ); \
        sleep(2); \
    } \
    while(0)

#define DISCONNECT_PERROR(fd, log) \
    do \
    { \
        perror( (log) ); \
        close( (fd) ); \
        sleep(2); \
    } \
    while(0)

/*
 * @return a file descriptor for the new socket, or -1 for errors.
 */
int create_socket()
{
    /* Create socket */
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0)
    {
        perror("Fail to create socket");
        return -1;
    }
    /* Set socket properties */
    int keep_alive = 1;
    int keep_time = 180;
    int keep_interval = 10;
    int keep_count = 3;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_KEEPALIVE, &keep_alive, sizeof(keep_alive)) < 0)
    {
        perror("Cannot set socket property SO_KEEPALIVE");
        return -1;
    }
    if (setsockopt(socket_fd, SOL_TCP, TCP_KEEPIDLE, &keep_time, sizeof(keep_time)) < 0)
    {
        perror("Cannot set socket property TCP_KEEPIDLE");
        return -1;
    }
    if (setsockopt(socket_fd, SOL_TCP, TCP_KEEPINTVL, &keep_interval, sizeof(keep_interval)) < 0)
    {
        perror("Cannot set socket property TCP_KEEPINTVL");
        return -1;
    }
    if (setsockopt(socket_fd, SOL_TCP, TCP_KEEPCNT, &keep_count, sizeof(keep_count)) < 0)
    {
        perror("Cannot set socket property TCP_KEEPCNT");
        return -1;
    }
    return socket_fd;
}


int main(int argc, char* argv[])
{
    int ret = 0;
    if (argc != 3)
    {
        puts("Usage : access_client server_address key");
        return 1;
    }
    int socket_fd;
    /* Set the server address */
    char* ip_str = strtok(argv[1], ":");
    char* port_str = strtok(NULL, ":");
    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(struct sockaddr_in));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(atoi(port_str));
    ret = inet_pton(AF_INET, ip_str, &server_address.sin_addr);
    if (ret <= 0)
    {
        puts("Error occurs when converting the IP string to bytes");
        return -1;
    }
    /* The information to be output on console */
    char info[128] = "Connecting to the server ";
    strcat(info, argv[1]);
    /* Connect the server*/
    for (;;)
    {
        socket_fd = create_socket();
        if (socket_fd < 0)
        {
            puts("Failed to create socket");
            return 1;
        }
        /* Connect the server */
        puts(info);
        ret = connect(socket_fd, (struct sockaddr*)&server_address, sizeof(struct sockaddr_in));
        if (ret < 0)
        {
            DISCONNECT_PERROR(socket_fd, "Cannot connect to server, reconnect later");
            continue;
        }
        puts("Connect server successfully");
        /* To be authenticated by the server */
        puts("Being authenticated by the server...");
        /* Try to send authentic data to the server
         * if failed to send the data, send it again
         * if failed for 3 times, reconnect to the server
         */
        int send_success = 0;
        char* key = argv[2];
        int i;
        for (i = 0; i < 3; i++)
        {
            ret = send(socket_fd, key, strlen(key) + 1, 0);
            if (ret > 0)
            {
                send_success = 1;
                break;
            }
        }
        if (!send_success)
        {
            DISCONNECT_PERROR(socket_fd, "Failed to send the authentic data to the server, reconnect later");
            continue;
        }
        char recv_buf[SOCKET_RECV_BUF_SIZE];
        ret = recv(socket_fd, recv_buf, sizeof(recv_buf), 0);
        if (ret > 0)
        {
            if (strcmp(recv_buf, "success") != 0)
            {
                DISCONNECT(socket_fd, "Authentic failed");
                return 1;
            }
        }
        else
        {
            DISCONNECT_PERROR(socket_fd, "Server disconnected, reconnect later");
            continue;
        }
        puts("Successfully authenticated");
        /* Receive data from the server */
        for (;;)
        {
            ret = recv(socket_fd, recv_buf, sizeof(recv_buf), 0);
            if (ret > 0)
            {
                printf("data received : %s\n", recv_buf);
                /* Send data back */
                ret = send(socket_fd, "success", sizeof("success"), 0);
                if (ret <= 0)
                {
                    DISCONNECT(socket_fd, "Server disconnected, reconnect later");
                    break;
                }
            }
            else
            {
                DISCONNECT(socket_fd, "Server disconnected, reconnect later");
                break;
            }
        }
    }
    return 0;
}
