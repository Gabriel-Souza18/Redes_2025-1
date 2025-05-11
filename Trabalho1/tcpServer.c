#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <openssl/md5.h>

#include "hashMd5.h"

#define BUFFER_SIZE 4096
#define MAX_CLIENTE_MSG 256

// Funções
int criarSocketServidor();
void configurarEnderecoServidor(struct sockaddr_in *server_addr, int porta);
void aguardarConexoes(int server_fd);
void tratarCliente(int client_sock, struct sockaddr_in client_addr);
int receberNomeArquivo(int client_sock, char *filename);
void enviarHashMd5(int client_sock, const char *filename);
void enviarArquivo(int client_sock, const char *filename);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <PORTA>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int porta = atoi(argv[1]);
    int server_fd = criarSocketServidor();

    struct sockaddr_in server_addr;
    configurarEnderecoServidor(&server_addr, porta);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Erro no bind");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 5) < 0) {
        perror("Erro no listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Servidor TCP escutando na porta %d...\n", porta);
    aguardarConexoes(server_fd);

    close(server_fd);
    return 0;
}

int criarSocketServidor() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Erro ao criar socket");
        exit(EXIT_FAILURE);
    }
    return sock;
}

void configurarEnderecoServidor(struct sockaddr_in *server_addr, int porta) {
    memset(server_addr, 0, sizeof(*server_addr));
    server_addr->sin_family = AF_INET;
    server_addr->sin_port = htons(porta);
    server_addr->sin_addr.s_addr = INADDR_ANY;
}

void aguardarConexoes(int server_fd) {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_sock;

    while (1) {
        client_sock = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        if (client_sock < 0) {
            perror("Erro no accept");
            continue;
        }

        printf("Conexão aceita de %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        tratarCliente(client_sock, client_addr);
    }
}

void tratarCliente(int client_sock, struct sockaddr_in client_addr) {
    char filename[MAX_CLIENTE_MSG];

    if (receberNomeArquivo(client_sock, filename) <= 0) {
        close(client_sock);
        return;
    }

    enviarHashMd5(client_sock, filename);
    enviarArquivo(client_sock, filename);

    close(client_sock);
    printf("Conexão com %s:%d encerrada.\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
}

int receberNomeArquivo(int client_sock, char *filename) {
    ssize_t len = recv(client_sock, filename, MAX_CLIENTE_MSG - 1, 0);
    if (len <= 0) {
        perror("Erro ao receber nome do arquivo");
        return -1;
    }
    filename[len] = '\0';
    printf("Cliente solicitou o arquivo: %s\n", filename);
    return 1;
}

void enviarHashMd5(int client_sock, const char *filename) {
    unsigned char hash[MD5_DIGEST_LENGTH];

    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        perror("Erro ao abrir arquivo para hash");
        return;
    }
    fclose(fp);

    calcularHashMd5(filename, hash);

    ssize_t sent = send(client_sock, hash, MD5_DIGEST_LENGTH, 0);
    if (sent < 0) {
        perror("Erro ao enviar hash");
    } else if (sent < MD5_DIGEST_LENGTH) {
        fprintf(stderr, "Aviso: hash enviado parcialmente.\n");
    }
}

void enviarArquivo(int client_sock, const char *filename) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        perror("Erro ao abrir arquivo para envio");
        return;
    }

    char buffer[BUFFER_SIZE];
    ssize_t bytes_read, bytes_sent;

    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, fp)) > 0) {
        bytes_sent = send(client_sock, buffer, bytes_read, 0);
        if (bytes_sent < 0) {
            perror("Erro ao enviar dados");
            break;
        }
        if (bytes_sent < bytes_read) {
            fprintf(stderr, "Aviso: envio parcial do arquivo.\n");
        }
    }

    if (ferror(fp)) {
        perror("Erro ao ler arquivo");
    } else {
        printf("Arquivo '%s' enviado com sucesso.\n", filename);
    }

    fclose(fp);
}
