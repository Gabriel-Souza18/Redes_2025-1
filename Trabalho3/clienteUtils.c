#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "clienteUtils.h"

void udp_client_init(UDPClient *client, const char *server_ip, int server_port) {
    client->socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (client->socket_fd < 0) {
        perror("Erro ao criar socket");
        exit(EXIT_FAILURE);
    }

    memset(&client->server_addr, 0, sizeof(client->server_addr));
    client->server_addr.sin_family = AF_INET;
    client->server_addr.sin_port = htons(server_port);
    if (inet_pton(AF_INET, server_ip, &client->server_addr.sin_addr) <= 0) {
        perror("Endereço inválido ou não suportado");
        exit(EXIT_FAILURE);
    }
}

int udp_client_send(UDPClient *client, const char *message, int message_len) {
    int bytes_sent = sendto(client->socket_fd, message, message_len, 0,
                            (struct sockaddr*)&client->server_addr, sizeof(client->server_addr));
    if (bytes_sent < 0) {
        perror("Erro ao enviar dados");
        return -1;
    }
    return bytes_sent;
}

int udp_client_receive(UDPClient *client, char *buffer, int buffer_size) {
    socklen_t addr_len = sizeof(client->server_addr);
    int bytes_received = recvfrom(client->socket_fd, buffer, buffer_size - 1, 0,
                                  (struct sockaddr*)&client->server_addr, &addr_len);
    if (bytes_received < 0) {
        perror("Erro ao receber dados");
        return -1;
    }
    buffer[bytes_received] = '\0';
    return bytes_received;
}

void udp_client_close(UDPClient *client) {
    close(client->socket_fd);
}