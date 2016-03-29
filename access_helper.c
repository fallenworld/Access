#include "access_helper.h"
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

int create_tcp_socket()
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

int open_localfd(char* file_name, int listen_queue_size)
{
    int ret;
    /* Create socket */
    int local_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (local_fd < 0)
    {
        perror("Fail to create local socket");
        return -1;
    }
    /* Set the socket address */
    unlink(file_name);
    struct sockaddr_un address;
    memset(&address, 0, sizeof(address));
    address.sun_family = AF_UNIX;
    strcpy(address.sun_path, file_name);
    /* bind */
    ret = bind(local_fd, (struct sockaddr*)&address, sizeof(address));
    if (ret < 0)
    {
        perror("Local socket bind failed");
        return -1;
    }
    /* listen */
    ret = listen(local_fd, listen_queue_size);
    if (ret < 0)
    {
        perror("Local socket listen failed");
        return -1;
    }
    return local_fd;
}

int open_listenfd(int port, int listen_queue_size)
{
    /* create socket */
    int listenfd = create_tcp_socket();
    if (listenfd < 0)
    {
        return -1;
    }
    /* set the address */
    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(struct sockaddr_in));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    /* bind and listen */
    if (bind(listenfd, (struct sockaddr*)&server_address, sizeof(struct sockaddr_in)) < 0)
    {
        perror("Socket bind failed");
        return -1;
    }
    if (listen(listenfd, listen_queue_size) < 0)
    {
        perror("Socket listen failed");
        return -1;
    }
}

int open_clientfd(const char* hostaddress)
{
    char* address_str = malloc(strlen(hostaddress) + 1);
    if (address_str == NULL)
    {
        return -1;
    }
    strcpy(address_str, hostaddress);
    /* create socket */
    int clientfd = create_tcp_socket();
    if (clientfd < 0)
    {
        free(address_str);
        close(clientfd);
        return -1;
    }
    /* Set the address */
    char* ip_str = strtok(address_str, ":");
    char* port_str = strtok(NULL, ":");
    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(struct sockaddr_in));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(atoi(port_str));
    if (inet_pton(AF_INET, ip_str, &server_address.sin_addr) <= 0)
    {
        free(address_str);
        puts("Error occurs when converting the IP string to bytes");
        close(clientfd);
        return -1;
    }
    /* Connect */
    if (connect(clientfd, (struct sockaddr*)&server_address, sizeof(struct sockaddr_in)) < 0)
    {
        free(address_str);
        perror("Cannot connect to server");
        close(clientfd);
        return -1;
    }
    return clientfd;
}

int accept_safe(int fd)
{
    int ret = accept(fd, NULL, NULL);
    if (ret < 0)
    {
        perror("Accept error");
        exit(1);
    }
    return ret;
}











