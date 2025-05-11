#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <openssl/md5.h> 
#include "hashMd5.h"      

#define BUFFER_SIZE 4096
#define MAX_CLIENTE_MSG 256

typedef struct {
    uint32_t numero_pacote;
    uint32_t tamanho_dados;
    char dados[BUFFER_SIZE - 8];
} Pacote;

// Funções
int criarSocketServidor();
void configurarEnderecoServidor(struct sockaddr_in *server_addr, int porta);
void aguardarSolicitacoes(int sockfd);
int receberNomeArquivo(int sockfd, char *nome_arquivo, struct sockaddr_in *client_addr, socklen_t *addr_len);
void enviarArquivo(const char *nome_arquivo, int sockfd, struct sockaddr_in *client_addr, socklen_t addr_len);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <PORTA>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int porta = atoi(argv[1]);
    int sockfd = criarSocketServidor();

    struct sockaddr_in server_addr;
    configurarEnderecoServidor(&server_addr, porta);

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Erro no bind");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Servidor UDP escutando na porta %d...\n", porta);
    aguardarSolicitacoes(sockfd);
    close(sockfd);
    return 0;
}

int criarSocketServidor() {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Erro ao criar socket UDP");
        exit(EXIT_FAILURE);
    }
    return sockfd;
}

void configurarEnderecoServidor(struct sockaddr_in *server_addr, int porta) {
    memset(server_addr, 0, sizeof(*server_addr));
    server_addr->sin_family = AF_INET;
    server_addr->sin_port = htons(porta);
    server_addr->sin_addr.s_addr = INADDR_ANY;
}

void aguardarSolicitacoes(int sockfd) {
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    char nome_arquivo[MAX_CLIENTE_MSG];

    while (1) {
        memset(nome_arquivo, 0, sizeof(nome_arquivo));
        if (receberNomeArquivo(sockfd, nome_arquivo, &client_addr, &addr_len) > 0) {
            enviarArquivo(nome_arquivo, sockfd, &client_addr, addr_len);
        }
    }
}

int receberNomeArquivo(int sockfd, char *nome_arquivo, struct sockaddr_in *client_addr, socklen_t *addr_len) {
    ssize_t bytes = recvfrom(sockfd, nome_arquivo, MAX_CLIENTE_MSG, 0,
                             (struct sockaddr *)client_addr, addr_len);
    if (bytes < 0) {
        perror("Erro ao receber nome do arquivo");
        return -1;
    }

    nome_arquivo[bytes] = '\0';
    printf("Cliente solicitou: %s\n", nome_arquivo);
    return 1;
}

void enviarArquivo(const char *nome_arquivo, int sockfd, struct sockaddr_in *client_addr, socklen_t addr_len) {
    FILE *fp = fopen(nome_arquivo, "rb");
    if (!fp) {
        perror("Arquivo não encontrado");
        return;
    }

    unsigned char hash[MD5_DIGEST_LENGTH];
    calcularHashMd5(nome_arquivo, hash);

    ssize_t sent = sendto(sockfd, hash, MD5_DIGEST_LENGTH, 0,
                          (struct sockaddr *)client_addr, addr_len);
    if (sent < 0) {
        perror("Erro ao enviar hash MD5");
        fclose(fp);
        return;
    } else if (sent < MD5_DIGEST_LENGTH) {
        fprintf(stderr, "Aviso: hash MD5 enviado parcialmente.\n");
    }

    Pacote pacote;
    uint32_t numero = 0;
    size_t lidos;

    while ((lidos = fread(pacote.dados, 1, sizeof(pacote.dados), fp)) > 0) {
        pacote.numero_pacote = numero;
        pacote.tamanho_dados = lidos;

        sendto(sockfd, &pacote, sizeof(pacote.numero_pacote) + sizeof(pacote.tamanho_dados) + lidos, 0,
               (struct sockaddr *)client_addr, addr_len);
        numero++;
    }

    // Pacote de fim
    pacote.numero_pacote = numero;
    pacote.tamanho_dados = 0;
    sendto(sockfd, &pacote, sizeof(pacote), 0,
           (struct sockaddr *)client_addr, addr_len);

    fclose(fp);
    printf("Arquivo %s enviado com sucesso (%u pacotes).\n", nome_arquivo, numero);
}
