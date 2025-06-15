#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "serverUtils.h"
#include "httpHandler.h"

#define PORTA 2020

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char client_ip[INET_ADDRSTRLEN];

    server_socket = create_server_socket(PORTA);
    server_addr = configure_server_address(PORTA);

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Erro no bind");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, 5) < 0) {
        perror("Erro no listen");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    //printf("Servidor iterativo ouvindo na porta %d...\n", PORTA);

    while (1) {
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
        if (client_socket < 0) {
            perror("Erro no accept");
            continue;
        }

        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
        //printf("Conexão recebida de %s:%d\n", client_ip, ntohs(client_addr.sin_port));

        handle_request(client_socket);

        close(client_socket);
    }

    close(server_socket);
    return 0;
}
