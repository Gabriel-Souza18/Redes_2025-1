#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define SERVER_PORT 8080	// Porta do servidor
#define BUFFER_SIZE 4096    // Tamanho total do buffer para enviar e receber dados
#define MAX_CLIENT_MSG 128  // Tamanho máximo da mensagem do cliente

typedef struct {
    uint32_t numero_pacote;
    uint32_t tamanho_dados;
    char dados[BUFFER_SIZE - 8];
} Pacote;

int criarSocketServidor();
void configurarEnderecoServidor(struct sockaddr_in *server_addr);
void aguardarSolicitacoes(int sockfd);
int receberNomeArquivo(int sockfd, char *nome_arquivo, struct sockaddr_in *client_addr, socklen_t *addr_len);
void enviarArquivo(const char *nome_arquivo, int sockfd, struct sockaddr_in *client_addr, socklen_t addr_len);

int main() {
    int sockfd = criarSocketServidor();

    struct sockaddr_in server_addr;
    configurarEnderecoServidor(&server_addr);

    // Vincular socket
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Erro no bind");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Servidor UDP aguardando solicitação na porta %d...\n", SERVER_PORT);

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

void configurarEnderecoServidor(struct sockaddr_in *server_addr) {
    memset(server_addr, 0, sizeof(*server_addr));
    server_addr->sin_family = AF_INET;
    server_addr->sin_port = htons(SERVER_PORT);
    server_addr->sin_addr.s_addr = INADDR_ANY;
}

void aguardarSolicitacoes(int sockfd) {
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    char nome_arquivo[MAX_CLIENT_MSG];

    while (1) {
        memset(nome_arquivo, 0, sizeof(nome_arquivo));

        if (receberNomeArquivo(sockfd, nome_arquivo, &client_addr, &addr_len) > 0) {
            enviarArquivo(nome_arquivo, sockfd, &client_addr, addr_len);
        }
    }
}

int receberNomeArquivo(int sockfd, char *nome_arquivo, struct sockaddr_in *client_addr, socklen_t *addr_len) {
    ssize_t bytes = recvfrom(sockfd, nome_arquivo, MAX_CLIENT_MSG, 0,
                             (struct sockaddr *)client_addr, addr_len);
    if (bytes < 0) {
        perror("Erro ao receber nome do arquivo");
        return -1;
    }

    nome_arquivo[bytes] = '\0'; // Garantir terminação
    printf("Cliente solicitou: %s\n", nome_arquivo);
    return 1;
}

void enviarArquivo(const char *nome_arquivo, int sockfd, struct sockaddr_in *client_addr, socklen_t addr_len) {
    FILE *fp = fopen(nome_arquivo, "rb");
    if (!fp) {
        perror("Arquivo não encontrado");
        return;
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
        usleep(200); // Simula perda ou controle de taxa
    }

    // Pacote final sinalizando fim
    pacote.numero_pacote = numero;
    pacote.tamanho_dados = 0;
    sendto(sockfd, &pacote, sizeof(pacote), 0,
           (struct sockaddr *)client_addr, addr_len);

    fclose(fp);
    printf("Arquivo %s enviado (%u pacotes).\n", nome_arquivo, numero);
}
