#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include "clienteUtils.h"

#define SERVER_PORT 8080
#define SERVER_IP "127.0.0.1"
#define BUFFER_SIZE 1024

int main() {
    UDPClient client;
    char buffer[BUFFER_SIZE];
    char message[BUFFER_SIZE];

    udp_client_init(&client, SERVER_IP, SERVER_PORT);


    // manda a mensagem para o servidor
    printf("Enter message to send to server: ");
    fgets(message, sizeof(message), stdin);
    message[strcspn(message, "\n")] = 0;

    if (udp_client_send(&client, message, strlen(message)) < 0) {
        fprintf(stderr, "Failed to send message\n");
        return EXIT_FAILURE;
    }

    // recebe a resposta do server
    int bytes_received = udp_client_receive(&client, buffer, sizeof(buffer));
    if (bytes_received < 0) {
        fprintf(stderr, "Failed to receive response\n");
        return EXIT_FAILURE;
    }

    printf("Received from server: %s\n", buffer);

    // fecha o cliente
    udp_client_close(&client);
    return EXIT_SUCCESS;
}