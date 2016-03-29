/*============================================================================
 * Name        : access_server.c
 * Author      : fallenworld
 * Version     : 0.5
 * Copyright   : All right is shit
 * Description : The access server
 ============================================================================*/
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <errno.h>
#include "access_helper.h"

#define SOCKET_LISTEN_QUEUE_SIZE 10
#define SOCKET_BUF_SIZE 512
#define SOCKET_FILE_PATH "./access_socket"

void checkRaspberryConnect(fd_set* ready_set, int tcp_fd);

void checkLocalConnect(fd_set* ready_set, int local_fd);

void checkRaspberryData(fd_set* ready_set);

void checkLocalData(fd_set* ready_set);

void handleIO(int tcp_fd, int local_fd);

void authenticateRaspberry();

int local_client_fd = -1;
int raspberry_fd = -1;
fd_set read_set;
char* key = NULL;

int main(int argc, char* argv[])
{
    int ret = 0;
    if (argc != 3)
    {
        puts("Usage : access_server server_port key\n");
        return 1;
    }
    /* start tcp listening */
    int tcp_fd = open_listenfd(atoi(argv[2]), SOCKET_LISTEN_QUEUE_SIZE);
    if (tcp_fd < 0)
    {
        exit(1);
    }
    /* start local socket */
    int local_fd = open_localfd(SOCKET_FILE_PATH, SOCKET_LISTEN_QUEUE_SIZE);
    if (local_fd < 0)
    {
        exit(1);
    }
    /* main loop */
    handleIO(tcp_fd, local_fd);
}


void handleIO(int tcp_fd, int local_fd)
{
    /* add tcp fd and local socket fd to read set */
    fd_set ready_set;
    FD_ZERO(&read_set);
    FD_SET(tcp_fd, &read_set);
    FD_SET(local_fd, &read_set);
    int ret;
    int max_fd = (tcp_fd > local_fd) ? tcp_fd : local_fd;
    /* main loop */
    for(;;)
    {
        ready_set = read_set;
        ret = select(max_fd + 1, &ready_set, NULL, NULL, NULL);
        if (ret < 0)
        {
            puts("Error occurs when executing select function");
            exit(1);
        }
        checkLocalConnect(&ready_set, local_fd);
        checkRaspberryConnect(&ready_set, tcp_fd);;
        checkLocalData(&ready_set);
        checkRaspberryData(&ready_set);
    }
}


void checkRaspberryConnect(fd_set* ready_set, int tcp_fd)
{
    if (raspberry_fd == -1 && FD_ISSET(tcp_fd, ready_set))
    {
        raspberry_fd = accept_safe(tcp_fd);
        if (raspberry_fd < 0)
        {
            raspberry_fd = -1;
            return;
        }
        authenticateRaspberry();
        if (raspberry_fd != -1)
        {
            FD_SET(raspberry_fd, &read_set);
        }
    }
}

void authenticateRaspberry()
{
    int ret;
    char buffer[SOCKET_BUF_SIZE];
    // get key from connected raspberry
    ret = recv(raspberry_fd, buffer, sizeof(buffer), 0);
    if (ret <= 0)
    {
        puts("Raspberry disconnected");
        return;
    }
    // check key
    if (strcmp(buffer, key) == 0) // received key matches the key on server
    {
        send(raspberry_fd, buffer, strlen("success") + 1, 0);
        puts("Raspberry authenticate successfully");
    }
    else // received key doesn't match the key on server
    {
        send(raspberry_fd, buffer, strlen("success") + 1, 0);
        puts("Raspberry authenticate failed");
        raspberry_fd = -1;
    }
}

void checkLocalConnect(fd_set* ready_set, int local_fd)
{
    if (local_client_fd == -1 && FD_ISSET(local_fd, ready_set))
    {
        local_client_fd = accept_safe(local_fd);
        if (local_client_fd < 0)
        {
            local_client_fd = -1;
            return;
        }
        FD_SET(local_client_fd, &read_set);
    }
}

void checkRaspberryData(fd_set* ready_set)
{
    if (raspberry_fd != -1 && FD_ISSET(raspberry_fd, ready_set))
    {
        int ret;
        char buffer[SOCKET_BUF_SIZE];
        // get data form raspberry
        ret = recv(raspberry_fd, buffer, sizeof(buffer), 0);
        if (ret <= 0)
        {
            puts("Raspberry disconnected");
            close(raspberry_fd);
            raspberry_fd = -1;
            return;
        }
        // send data to weixin
        if (local_client_fd != -1)
        {
            ret = send(local_client_fd, buffer, sizeof(buffer), 0);
            if (ret <= 0)
            {
                puts("WeiXin disconnected");
                close(local_client_fd);
                local_client_fd = -1;
                return;
            }
        }
    }

}

void checkLocalData(fd_set* ready_set)
{
    if (local_client_fd != -1 && FD_ISSET(local_client_fd, ready_set))
    {
        int ret;
        char buffer[SOCKET_BUF_SIZE];
        // get data form the weixin
        ret = recv(local_client_fd, buffer, sizeof(buffer), 0);
        if (ret <= 0)
        {
            puts("WeiXin program disconnected");
            close(local_client_fd);
            local_client_fd = -1;
            return;
        }
        // send data to raspberry
        if (raspberry_fd != -1)
        {
            ret = send(raspberry_fd, buffer, sizeof(buffer), 0);
            if (ret <= 0)
            {
                puts("Raspberry disconnected");
                close(raspberry_fd);
                raspberry_fd = -1;
                return;
            }
        }
        else // if raspberry is neither connected nor authenticated, send fail message to weixin
        {
            char* info = "fail:1";
            ret = send(local_client_fd, info, strlen(info) + 1, 0);
            if (ret <= 0)
            {
                puts("WeiXin program disconnected");
                close(local_client_fd);
                local_client_fd = -1;
                return;
            }
        }
    }
}
