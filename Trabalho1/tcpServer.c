#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h> // Para sockaddr_in
#include <arpa/inet.h> // Para inet_ntoa
#include <openssl/md5.h> // Para MD5_DIGEST_LENGTH

#include "hashMd5.h" // calcularHashMd5 


#define SERVER_PORT 8081      // Porta para escutar
#define BUFFER_SIZE 4096      // Tamanho do bloco (4KB)
#define MAX_FILENAME_SIZE 256 // Tamanho máximo do nome do arquivo


int main() {
    int server_fd, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    char file_buffer[BUFFER_SIZE]; // Buffer para o conteúdo do arquivo
    unsigned char hash_buffer[MD5_DIGEST_LENGTH]; // Buffer dedicado para o hash
    char filename[MAX_FILENAME_SIZE];
    FILE *fp;
    ssize_t bytes_read, bytes_sent, filename_len;

    // Criar socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Erro ao criar socket");
        exit(EXIT_FAILURE);
    }

    // Configurar endereço do servidor
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(SERVER_PORT);

    // Bind
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Erro no bind");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Listen
    if (listen(server_fd, 5) < 0) { // Permitir até 5 conexões pendentes
        perror("Erro no listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    printf("Servidor escutando na porta %d...\n", SERVER_PORT);

    while (1) { // Loop para aceitar múltiplas conexões (simplificado)
        // Accept
        client_sock = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_sock < 0) {
            perror("Erro no accept");
            continue; // Tenta aceitar a próxima conexão
        }
        printf("Conexão aceita de %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        // Receber nome do arquivo do cliente
        memset(filename, 0, MAX_FILENAME_SIZE); // Limpar buffer do nome do arquivo
        filename_len = recv(client_sock, filename, MAX_FILENAME_SIZE - 1, 0);
        if (filename_len <= 0) {
            perror("Erro ao receber nome do arquivo ou cliente desconectou");
            close(client_sock);
            continue;
        }
        filename[filename_len] = '\0'; // Garantir terminação nula
        printf("Cliente solicitou o arquivo: %s\n", filename);

        // --- Etapa 1: Calcular o hash do arquivo solicitado ---
        // Primeiro, verificar se o arquivo existe antes de calcular o hash
        fp = fopen(filename, "rb");
        if (fp == NULL) {
            perror("Erro ao abrir arquivo solicitado para cálculo do hash");
            close(client_sock);
        }
        fclose(fp); 

        // Calcular o hash MD5 do arquivo
        calcularHashMd5(filename, hash_buffer);
        printf("Hash MD5 do arquivo '%s' calculado.\n", filename);

        // --- Etapa 2: Enviar o hash MD5 primeiro ---
        printf("Enviando hash MD5 (%d bytes) para o cliente...\n", MD5_DIGEST_LENGTH);
        bytes_sent = send(client_sock, hash_buffer, MD5_DIGEST_LENGTH, 0);
        if (bytes_sent < 0) {
            perror("Erro ao enviar hash");
            close(client_sock);
            continue; // Próxima conexão
        }
        if (bytes_sent < MD5_DIGEST_LENGTH) {
             fprintf(stderr, "Aviso: Nem todos os bytes do hash foram enviados.\n");
        }
         printf("Hash MD5 enviado com sucesso.\n");

        // --- Etapa 3: Abrir e enviar o arquivo original ---
        fp = fopen(filename, "rb"); // Reabre o arquivo para leitura

        // Ler arquivo em blocos e enviar ao cliente
        printf("Enviando arquivo '%s'...\n", filename);
        memset(file_buffer, 0, BUFFER_SIZE); // Limpar buffer de envio do arquivo
        while ((bytes_read = fread(file_buffer, 1, BUFFER_SIZE, fp)) > 0) {
            // Enviar bloco lido ao cliente
            bytes_sent = send(client_sock, file_buffer, bytes_read, 0);
            if (bytes_sent < 0) {
                perror("Erro ao enviar dados do arquivo");
                break; // Sai do loop de envio se houver erro
            }
            if (bytes_sent < bytes_read) {
                 fprintf(stderr, "Aviso: Nem todos os bytes do arquivo foram enviados na última operação send().\n");
                 // Poderia adicionar lógica para reenviar o restante
            }
        }

        if (ferror(fp)) {
            perror("Erro ao ler o arquivo");
        } else {
            printf("Arquivo '%s' enviado com sucesso.\n", filename);
        }

        // --- Etapa 4: Limpeza ---
        fclose(fp); // Fecha o arquivo original
        close(client_sock); // Fecha a conexão com o cliente APÓS enviar hash e arquivo
        printf("Conexão com %s:%d fechada.\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    }

    // Fechar socket do servidor (nunca alcançado neste loop infinito)
    close(server_fd);

    return 0;
}