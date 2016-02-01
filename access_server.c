/*============================================================================
 * Name        : access_server.cpp
 * Author      : fallenworld
 * Version     : 0.1
 * Copyright   :
 * Description : The access server
 ============================================================================*/
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#define SOCKET_LISTEN_QUEUE_SIZE 10
#define FIFO_READ_BUFFER_SIZE 128
#define SOCKET_RECV_BUF_SIZE 512
#define AUTHENTIC_KEY "xiaoqingxinzuiqingxin"

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
 * create a FIFO and start to watch it
 */
void start_fifo(int client_socket_fd)
{
    const char* fifo_name = "fifo.tmp";
    if (mkfifo(fifo_name, 0777) != 0)
    {
    	puts("Cannot create FIFO");
    	return;
    }
    int fd;
    char read_buf[FIFO_READ_BUFFER_SIZE];
    while (1)
    {
    	fd = open(fifo_name, O_RDONLY);
    	read(fd, read_buf, FIFO_READ_BUFFER_SIZE);
    	close(fd);
    	if (send(client_socket_fd, read_buf, strlen(read_buf) + 1, 0) < 0)
    	{
    		return ;
    	}

    }
}

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        puts("Usage:access_server server_port\n");
        return 1;
    }
    int socket_fd = create_socket();
    if (socket_fd < 0)
    {
        puts("Failed to create socket");
        return 1;
    }
    /* Bind to port */
    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(struct sockaddr_in));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(atoi(argv[1]));
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(socket_fd, (struct sockaddr*)&server_address, sizeof(struct sockaddr_in)) < 0)
    {
        perror("Socket bind failed");
        return 1;
    }
    /* Listen */
    if (listen(socket_fd, SOCKET_LISTEN_QUEUE_SIZE) < 0)
    {
        perror("Socket listen failed");
        return 1;
    }
    /* Main loop, wait for a client to connect */
    int client_socket_fd;
    char recv_buf[SOCKET_RECV_BUF_SIZE];
    while(1)
    {
    	client_socket_fd = accept(socket_fd, NULL, NULL);
    	if (client_socket_fd < 0)
    	{
    		close(client_socket_fd);
    		continue;
    	}
        puts("Client connected");
        if (recv(client_socket_fd, recv_buf, sizeof(recv_buf), 0) < 0)
        {
            perror("Client disconnected");
        	close(client_socket_fd);
    		continue;
        }
        char authentic_return[8];
        if (strcmp(recv_buf, AUTHENTIC_KEY) == 0)
        {
        	if (send(client_socket_fd, "success", 8, 0) < 0)
        	{
                perror("Client disconnected");
            	close(client_socket_fd);
        		continue;
        	}
            puts("Client authenticate successfully");
            start_fifo(client_socket_fd);
            perror("Client disconnected");
        	close(client_socket_fd);
        }
        else
        {
        	send(client_socket_fd, "fail", 5, 0);
            puts("Client authenticate failed");
            close(client_socket_fd);
        }

    }
    return 0;
}
