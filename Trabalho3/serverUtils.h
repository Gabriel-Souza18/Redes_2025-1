#ifndef SERVER_UTILS_H
#define SERVER_UTILS_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_BUFFER_SIZE 1024
#define DEFAULT_PORT 8080

typedef struct {
    int socket_fd;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    socklen_t client_len;
    int port;
    int is_open;
} UDPServer;

// Funções básicas do servidor UDP
int udp_server_open(UDPServer *server, int port);
int udp_server_close(UDPServer *server);
int udp_server_receive(UDPServer *server, char *buffer, int buffer_size);
int udp_server_send(UDPServer *server, const char *message, int message_len);
int udp_server_send_to_client(UDPServer *server, const char *message, int message_len, struct sockaddr_in *client);
void udp_server_init(UDPServer *server);
void udp_server_print_info(UDPServer *server);

#endif