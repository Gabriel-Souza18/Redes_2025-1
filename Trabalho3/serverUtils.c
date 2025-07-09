#include "serverUtils.h"

void udp_server_init(UDPServer *server) {
    server->socket_fd = -1;
    server->port = DEFAULT_PORT;
    server->is_open = 0;
    server->client_len = sizeof(server->client_addr);
    memset(&server->server_addr, 0, sizeof(server->server_addr));
    memset(&server->client_addr, 0, sizeof(server->client_addr));
}

int udp_server_open(UDPServer *server, int port) {
    if (server->is_open) {
        printf("Servidor já está aberto\n");
        return -1;
    }

    // Criar socket UDP
    server->socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (server->socket_fd < 0) {
        perror("Erro ao criar socket");
        return -1;
    }

    // Configurar endereço do servidor
    server->server_addr.sin_family = AF_INET;
    server->server_addr.sin_addr.s_addr = INADDR_ANY;
    server->server_addr.sin_port = htons(port);
    server->port = port;

    // Fazer bind do socket
    if (bind(server->socket_fd, (struct sockaddr*)&server->server_addr, 
             sizeof(server->server_addr)) < 0) {
        perror("Erro no bind");
        close(server->socket_fd);
        return -1;
    }

    server->is_open = 1;
    printf("Servidor UDP aberto na porta %d\n", port);
    return 0;
}

int udp_server_close(UDPServer *server) {
    if (!server->is_open) {
        printf("Servidor não está aberto\n");
        return -1;
    }

    if (close(server->socket_fd) < 0) {
        perror("Erro ao fechar socket");
        return -1;
    }

    server->is_open = 0;
    server->socket_fd = -1;
    printf("Servidor UDP fechado\n");
    return 0;
}

int udp_server_receive(UDPServer *server, char *buffer, int buffer_size) {
    if (!server->is_open) {
        printf("Servidor não está aberto\n");
        return -1;
    }

    server->client_len = sizeof(server->client_addr);
    int bytes_received = recvfrom(server->socket_fd, buffer, buffer_size - 1, 0,
                                  (struct sockaddr*)&server->client_addr, 
                                  &server->client_len);
    
    if (bytes_received < 0) {
        perror("Erro ao receber dados");
        return -1;
    }

    buffer[bytes_received] = '\0';
    return bytes_received;
}

int udp_server_send(UDPServer *server, const char *message, int message_len) {
    return udp_server_send_to_client(server, message, message_len, &server->client_addr);
}

int udp_server_send_to_client(UDPServer *server, const char *message, int message_len, 
                              struct sockaddr_in *client) {
    if (!server->is_open) {
        printf("Servidor não está aberto\n");
        return -1;
    }

    int bytes_sent = sendto(server->socket_fd, message, message_len, 0,
                           (struct sockaddr*)client, sizeof(*client));
    
    if (bytes_sent < 0) {
        perror("Erro ao enviar dados");
        return -1;
    }

    return bytes_sent;
}

void udp_server_print_info(UDPServer *server) {
    printf("=== Informações do Servidor UDP ===\n");
    printf("Status: %s\n", server->is_open ? "Aberto" : "Fechado");
    printf("Porta: %d\n", server->port);
    printf("Socket FD: %d\n", server->socket_fd);
    
    if (server->is_open) {
        printf("Endereço: %s:%d\n", 
               inet_ntoa(server->server_addr.sin_addr), 
               ntohs(server->server_addr.sin_port));
    }
    printf("===================================\n");
}