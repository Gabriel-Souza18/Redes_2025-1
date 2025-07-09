#ifndef CLIENT_UTILS_H
#define CLIENT_UTILS_H

#include <netinet/in.h>

#define DEFAULT_PORT 8080
#define BUFFER_SIZE 1024

typedef struct {
    int socket_fd;
    struct sockaddr_in server_addr;
} UDPClient;

void udp_client_init(UDPClient *client, const char *server_ip, int port);
int udp_client_send(UDPClient *client, const char *message, int message_len);
int udp_client_receive(UDPClient *client, char *buffer, int buffer_size);
void udp_client_close(UDPClient *client);

#endif // CLIENT_UTILS_H