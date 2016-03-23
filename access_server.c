/*============================================================================
 * Name        : access_server.c
 * Author      : fallenworld
 * Version     : 0.1
 * Copyright   :
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
#define AUTHENTIC_SUCCESS_RETURN "success"
#define AUTHENTIC_FAIL_RETURN "fail"
#define SOCKET_FILE_PATH "./access_socket"

void checkRaspberryConnect(fd_set* ready_set, int tcp_fd);

void checkLocalConnect(fd_set* ready_set, int local_fd);

void checkRaspberryData(fd_set* ready_set);

void checkLocalData(fd_set* ready_set);

void handleIO(int tcp_fd, int local_fd);

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
    fd_set read_set;
    fd_set ready_set;
    FD_ZERO(&read_set);
    FD_SET(tcp_fd, &read_set);
    FD_SET(local_fd, &read_set);
    int ret;
    int maxfd = (tcp_fd > local_fd) ? tcp_fd : local_fd;
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
        checkRaspberryConnect(&ready_set, tcp_fd);
        checkLocalConnect(&ready_set, local_fd);
        checkRaspberryData(&ready_set);
        checkLocalData(&ready_set);
    }
}


void checkRaspberryConnect(fd_set* ready_set, int tcp_fd)
{

}

void checkLocalConnect(fd_set* ready_set, int local_fd)
{

}

void checkRaspberryData(fd_set* ready_set)
{

}

void checkLocalData(fd_set* ready_set)
{

}



    /* Main loop, wait for the raspberry to connect */
    char recv_buf[SOCKET_BUF_SIZE];
    int client_socket_fd = 0;
    for(;;)
    {
        memset(recv_buf, 0, sizeof(recv_buf));
        client_socket_fd = accept(socket_fd, NULL, NULL);
        if (client_socket_fd < 0)
        {
            close(client_socket_fd);
            continue;
        }
        puts("Raspberry connected");
        /* Receive authentic data form raspberry */
        ret = recv(client_socket_fd, recv_buf, sizeof(recv_buf), 0);
        if (ret < 0)
        {
            perror("Raspberry disconnected");
            close(client_socket_fd);
            continue;
        }
        char authentic_return[8];
        memset(authentic_return, 0, sizeof(authentic_return));
        /* Check if authentic data is OK */
        /* If authentic success */
        char* key = argv[2];
        if (strcmp(recv_buf, key) == 0)
        {
            ret = send(client_socket_fd, AUTHENTIC_SUCCESS_RETURN, sizeof(AUTHENTIC_SUCCESS_RETURN), 0);
            if (ret <= 0)
            {
                perror("Raspberry disconnected");
                close(client_socket_fd);
                continue;
            }
            puts("Raspberry authenticate successfully");
            pthread_mutex_lock(&connected_lock);
            client_connected = 1;
            pthread_mutex_unlock(&connected_lock);
            /* Set timeout */
            struct timeval timeout = {4, 0};
            setsockopt(client_socket_fd, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
            /* Wait data from local socket */
            for (;;)
            {
                pthread_mutex_lock(&request_lock);
                while (request_buf[0] == 0)
                {
                	pthread_cond_wait(&request_cond, &request_lock);
                }
                pthread_mutex_unlock(&request_lock);
                printf("Send \"%s\" to raspberry\n", request_buf);
                /* Send data to raspberry*/
                ret = send(client_socket_fd, request_buf, strlen(request_buf) + 1, 0);
                if (ret < 0)
                {
                    perror("Raspberry disconnected");
                    close(client_socket_fd);
                    break;
                }
                memset(request_buf, 0, SOCKET_BUF_SIZE);
                /* Receive data from raspberry*/
                memset(recv_buf, 0, sizeof(recv_buf));
                ret = recv(client_socket_fd, recv_buf, sizeof(recv_buf), 0);
                char* response;
                if (ret <= 0)
                {
                	if (errno == EAGAIN)
                	{
                		perror("Receiving data from raspberry timeout");
                		response = "fail:timeout";
                	}
                	else
                	{
                        perror("Raspberry disconnected");
                        close(client_socket_fd);
                        break;
                	}
                }
                else
                {
                	response = recv_buf;
                }
                /* Wake up the local socket thread */
                pthread_mutex_lock(&response_lock);
                strcpy(response_buf, response);
                pthread_cond_signal(&response_cond);
                pthread_mutex_unlock(&response_lock);
            }
            pthread_mutex_lock(&connected_lock);
            client_connected = 1;
            pthread_mutex_unlock(&connected_lock);
        }
        /* If authentic fail */
        else
        {
            send(client_socket_fd, AUTHENTIC_FAIL_RETURN, sizeof(AUTHENTIC_FAIL_RETURN), 0);
            puts("Raspberry authenticate failed");
            close(client_socket_fd);
        }
    }
    ret = pthread_join(local_socket_thread, NULL);
    if (ret != 0)
    {
        puts("Cannot join thread");
        return 1;
    }
    return 0;














