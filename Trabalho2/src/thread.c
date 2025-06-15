#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <string.h>

#include "serverUtils.h"
#include "httpHandler.h"

#define PORTA 2022

typedef struct {
    int client_socket;
    struct sockaddr_in client_addr;
} client_data_t;

void* thread_func(void* arg) {
    client_data_t* data = (client_data_t*)arg;

    char ip_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(data->client_addr.sin_addr), ip_str, sizeof(ip_str));
 //   printf("Conexão recebida de %s:%d (thread)\n", ip_str, ntohs(data->client_addr.sin_port));

    handle_request(data->client_socket);

    close(data->client_socket);
    free(data);  // Liberar memória da estrutura passada
    pthread_exit(NULL);
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    server_socket = create_server_socket(PORTA);
    server_addr = configure_server_address(PORTA);

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Erro no bind");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, 10) < 0) {
        perror("Erro no listen");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

//    printf("Servidor concorrente (pthread) ouvindo na porta %d...\n", PORTA);

    while (1) {
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
        if (client_socket < 0) {
            perror("Erro no accept");
            continue;
        }

        client_data_t* data = malloc(sizeof(client_data_t));
        if (!data) {
            perror("Erro de alocação");
            close(client_socket);
            continue;
        }

        data->client_socket = client_socket;
        data->client_addr = client_addr;

        pthread_t tid;
        if (pthread_create(&tid, NULL, thread_func, data) != 0) {
            perror("Erro ao criar thread");
            close(client_socket);
            free(data);
        } else {
            pthread_detach(tid);  // Libera os recursos da thread automaticamente
        }
    }

    close(server_socket);
    return 0;
}
