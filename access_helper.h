/*
 * Create a new socket for remote use
 * @return a file descriptor for the new socket, or -1 for errors.
 */
int create_tcp_socket();

/*
 * create a socket to listen
 * @param port the port to be listened
 * @return the file descriptor for the socket, or the process will exit for errors
 */
int open_listenfd(int port, int listen_queue_size);

/*
 * start the local socket to listen
 * @param file_name unix socket address
 * @return the file descriptor for the socket, or the process will exit for errors
 */
int open_localfd(char* file_name, int listen_queue_size);

/*
 * create a client socket
 * @param hostaddress the host address, the format of this string is "ip:port"
 * @return the file descriptor for the socket, or -1 for error
 */
int open_clientfd(const char* hostaddress);

/*
 * the accept function wrapper
 */
int accept_safe(int fd, sockaddr* address, socklen_t* size);
