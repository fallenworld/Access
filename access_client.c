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
#include "access_helper.h"
#include "access_commands.h"

/* to be authenticated by the server
 * @param file desceipter
 * @param key the authentic key
 * @return return 0 for success, 1 for failure, -1 for network errors */
int authenticate(int fd, char* key);

/* handle a connection
 * @param fd the socket file descripter */
void handle_connection(int fd);

int main(int argc, char* argv[])
{
    int ret = 0;
    /* check arguments */
    if (argc != 3)
    {
        puts("Usage : access_client server_address key");
        return 1;
    }
    for (;;)
    {
        /* connect to the server*/
        printf("Connecting to the server %s", argv[1]);
        int fd = open_clientfd(argv[1]);
        if (fd < 0)
        {
            continue;
        }
        puts("Connect server successfully");
        /* authentic */
        puts("Being authenticated by the server...");
        ret = authenticate(fd, argv[2]);
        if (ret < 0)
        {
            continue;
        }
        else if (ret == 1)
        {
            exit(1);
        }
        else if (ret == 0)
        {
            handle_connection(fd);
        }
    }
    return 0;
}


int authenticate(int fd, char* key)
{
    /* send the authetic data */
    if (send(fd, key, strlen(key) + 1, 0) <= 0)
    {
        perror("Cannot send authentic data to server, disconnect");
        close(fd);
        return -1;
    }
    /* receive the return message from server */
    char recv_buf[16];
    if (recv(fd, recv_buf, sizeof(recv_buf), 0) > 0)
    {
        if (strcmp(recv_buf, "success") == 0)
        {
            /* the key is correct */
            puts("Authentic successfully");
            return 0;
        }
        else
        {
            /* the key is incorrect */
            puts("Authentic fail");
            return 1;
        }
    }
    else
    {
        perror("Cannot receive data to server, disconnect");
        close(fd);
        return -1;
    }
}

void handle_connection(int fd)
{
    int ret;
    char recv_buf[256];
    for (;;)
    {
        ret = recv(fd, recv_buf, sizeof(recv_buf), 0);
        if (ret > 0)
        {
            printf("data received : %s\n", recv_buf);
            char* info;
            int return_code = 0;
            if (strcmp(recv_buf, "open") == 0)
            {
                return_code = openDoor();
            }
            else if (strcmp(recv_buf, "close") == 0)
            {
                return_code = closeDoor();
            }
            else if (strcmp(recv_buf, "getState") == 0)
            {
            	return_code = getDoorState();
            }
            /* Send data back */
            ret = send(fd, "success", sizeof("success"), 0);
            if (ret <= 0)
            {
                puts("Cannot send data to server, disconnect");
                close(fd);
                break;
            }
        }
        else
        {
            puts("Cannot receive data from server, disconnect");
            close(fd);
            break;
        }
    }
}
