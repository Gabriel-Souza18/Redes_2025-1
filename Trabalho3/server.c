#include "serverUtils.h"
#include "utils.h"

int main() {
    UDPServer server;
    udp_server_init(&server);
    
    int port = 8080;

    char BUFFER[MAX_BUFFER_SIZE];
    udp_server_open(&server, port);

    // Loop para receber mensagens
    printf("Servidor UDP rodando na porta %d...\n", port);
    logger("Servidor UDP iniciado", "server.log");

    udp_server_print_info(&server);
    while (1) { 
        int bytes_received = udp_server_receive(&server, BUFFER, sizeof(BUFFER));
        if (bytes_received > 0) {
            BUFFER[bytes_received] = '\0'; 
            logger(BUFFER, "server.log");
            printf("Mensagem recebida: %s\n", BUFFER);
            
            // Enviar resposta de volta ao cliente
            const char *response = "Mensagem recebida com sucesso!";
            logger(response, "server.log");
            udp_server_send_to_client(&server, response, strlen(response), &server.client_addr);
        }
    }
    udp_server_close(&server);
    logger("Servidor UDP fechado", "server.log");
    return 0;
}