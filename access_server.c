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

// GPIO head file on raspberry
#ifdef TARGET_RASPBERRY
#include <>
#endif

#define SOCKET_LISTEN_QUEUE_SIZE 10
#define SOCKET_BUF_SIZE 512
#define AUTHENTIC_SUCCESS_RETURN "success"
#define AUTHENTIC_FAIL_RETURN "fail"
#define SOCKET_FILE_PATH "./access_socket"

char request_buf[SOCKET_BUF_SIZE];
char response_buf[SOCKET_BUF_SIZE];
int client_connected = 0;
pthread_mutex_t connected_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t response_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t request_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t response_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t request_cond = PTHREAD_COND_INITIALIZER;

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

/*
 * create a local socket to receive data from WeiXin
 */
void* start_local_socket(void* arg)
{
    int ret;
    /* Create socket */
    int local_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (local_fd < 0)
    {
        perror("Fail to create local socket");
        exit(1);
    }
    /* Set the socket address */
    unlink(SOCKET_FILE_PATH);
    struct sockaddr_un address;
    memset(&address, 0, sizeof(address));
    address.sun_family = AF_UNIX;
    strcpy(address.sun_path, SOCKET_FILE_PATH);
    /* bind */
    ret = bind(local_fd, (struct sockaddr*)&address, sizeof(address));
    if (ret < 0)
    {
        perror("Local socket bind failed");
        exit(1);
    }
    /* listen */
    ret = listen(local_fd, SOCKET_LISTEN_QUEUE_SIZE);
    if (ret < 0)
    {
        perror("Local socket listen failed");
        exit(1);
    }
    /* main loop, handle data from WeiXin */
    int wx_fd = 0;
    for (;;)
    {
        /* Wait WeiXin server to connect */
    	wx_fd = accept(local_fd, NULL, NULL);
        if (wx_fd < 0)
        {
            close(wx_fd);
            continue;
        }
        puts("WeiXin server connected");
        char recv_buf[SOCKET_BUF_SIZE];
        for(;;)
        {
            memset(recv_buf, 0, sizeof(recv_buf));
            /* Receive data from WeiXin */
            ret = recv(wx_fd, recv_buf, SOCKET_BUF_SIZE, 0);
            if (ret <= 0)
            {
                perror("WeiXin server disconnected");
                close(wx_fd);
                break;
            }
            int is_client_connected;
            pthread_mutex_lock(&connected_lock);
            is_client_connected = client_connected;
            pthread_mutex_unlock(&connected_lock);
            char* response;
            if (is_client_connected)
            {
                /* Send data to local */
                pthread_mutex_lock(&request_lock);
                strcpy(request_buf, recv_buf);
                pthread_mutex_unlock(&request_lock);
                pthread_cond_signal(&request_cond);
                /* Receive data from local */
                pthread_mutex_lock(&response_lock);
                while (response_buf[0] == 0)
                {
                	pthread_cond_wait(&response_cond, &response_lock);
                }
                pthread_mutex_unlock(&response_lock);
                response = response_buf;
            }
            else
            {
            	response = "fail:raspberry disconnected";
            }
            /* Send data to WeiXin */
            ret = send(wx_fd, response, strlen(response) + 1, 0);
            if (ret < 0)
            {
                perror("WeiXin server disconnected");
                close(wx_fd);
                break;
            }
            memset(response_buf, 0, SOCKET_BUF_SIZE);
        }
    }
    return NULL;
}

int main(int argc, char* argv[])
{
    int ret = 0;
    if (argc != 3)
    {
        puts("Usage : access_server server_port key\n");
        return 1;
    }
    memset(request_buf, 0 ,SOCKET_BUF_SIZE);
    memset(response_buf, 0 ,SOCKET_BUF_SIZE);
    /* Thread which runs local socket */
    pthread_t local_socket_thread;
    ret = pthread_create(&local_socket_thread, NULL, start_local_socket, NULL);
    if (ret != 0)
    {
        puts("Cannot create thread");
        return 1;
    }
    /* Create socket */
    int socket_fd = create_socket();
    if (socket_fd < 0)
    {
        puts("Failed to create remote socket");
        return 1;
    }
    /* Set the socket address */
    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(struct sockaddr_in));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(atoi(argv[1]));
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    /* Bind to port */
    ret = bind(socket_fd, (struct sockaddr*)&server_address, sizeof(struct sockaddr_in));
    if ( ret < 0)
    {
        perror("Remote Socket bind failed");
        return 1;
    }
    /* Listen */
    ret = listen(socket_fd, SOCKET_LISTEN_QUEUE_SIZE);
    if (ret < 0)
    {
        perror("Remote Socket listen failed");
        return 1;
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
}
