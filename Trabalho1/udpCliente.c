#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <time.h>
#include <sys/stat.h>
#include <errno.h>
#include <openssl/md5.h>
#include "hashMd5.h"
#include <openssl/evp.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080
#define BUFFER_SIZE 4096
#define RECEIVED_DIR "Recebidos"
#define HASH_FILENAME "hashRecebido.txt"
#define SAVE_FILENAME "arquivo_recebido.txt"
#define MAX_PACKETS 100000

typedef struct {
    uint32_t numero_pacote;
    uint32_t tamanho_dados;
    char dados[BUFFER_SIZE - 8];
} Pacote;

// ASSINATURAS DAS FUNÇÕES AUXILIARES
void criarDiretorioSeNecessario();
FILE *abrirArquivoRecebido(char *filepath);
int criarSocketCliente(struct sockaddr_in *server_addr);
void enviarSolicitacao(int sock, const char *nome_arquivo, struct sockaddr_in *server_addr);
int receberPacotes(int sock, FILE *fp, struct sockaddr_in *server_addr, int *recebido, int *maior_pacote);
void calcularEMostrarEstatisticas(struct timespec inicio, struct timespec fim, int maior_pacote, int total_recebidos, const char *filepath);

// FUNÇÃO PRINCIPAL
int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <nome_arquivo>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr;
    int sock;
    FILE *fp;
    int recebido[MAX_PACKETS] = {0};
    int total_recebidos = 0, maior_pacote = 0;
    struct timespec inicio, fim;
    char filepath[256];

    criarDiretorioSeNecessario();

    snprintf(filepath, sizeof(filepath), "%s/%s", RECEIVED_DIR, SAVE_FILENAME);
    fp = abrirArquivoRecebido(filepath);
    sock = criarSocketCliente(&server_addr);
    enviarSolicitacao(sock, argv[1], &server_addr);

    clock_gettime(CLOCK_MONOTONIC, &inicio);
    total_recebidos = receberPacotes(sock, fp, &server_addr, recebido, &maior_pacote);
    clock_gettime(CLOCK_MONOTONIC, &fim);

    fclose(fp);
    close(sock);

    calcularEMostrarEstatisticas(inicio, fim, maior_pacote, total_recebidos, filepath);

    return 0;
}

// IMPLEMENTAÇÕES DAS FUNÇÕES AUXILIARES

void criarDiretorioSeNecessario() {
    struct stat st = {0};
    if (stat(RECEIVED_DIR, &st) == -1) {
        mkdir(RECEIVED_DIR, 0700);
    }
}

FILE *abrirArquivoRecebido(char *filepath) {
    FILE *fp = fopen(filepath, "wb");
    if (!fp) {
        perror("Erro ao criar arquivo");
        exit(EXIT_FAILURE);
    }
    return fp;
}

int criarSocketCliente(struct sockaddr_in *server_addr) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("Erro ao criar socket");
        exit(EXIT_FAILURE);
    }

    memset(server_addr, 0, sizeof(*server_addr));
    server_addr->sin_family = AF_INET;
    server_addr->sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr->sin_addr);

    return sock;
}

void enviarSolicitacao(int sock, const char *nome_arquivo, struct sockaddr_in *server_addr) {
    sendto(sock, nome_arquivo, strlen(nome_arquivo), 0,
           (struct sockaddr *)server_addr, sizeof(*server_addr));
}

int receberPacotes(int sock, FILE *fp, struct sockaddr_in *server_addr, int *recebido, int *maior_pacote) {
    Pacote pacote;
    socklen_t addr_len = sizeof(*server_addr);
    ssize_t bytes_recebidos;
    int total_recebidos = 0;

    while (1) {
        bytes_recebidos = recvfrom(sock, &pacote, sizeof(pacote), 0,
                                   (struct sockaddr *)server_addr, &addr_len);
        if (bytes_recebidos < 0) break;

        if (pacote.tamanho_dados == 0) break;

        if (pacote.numero_pacote >= MAX_PACKETS) continue;

        fseek(fp, pacote.numero_pacote * (BUFFER_SIZE - 8), SEEK_SET);
        fwrite(pacote.dados, 1, pacote.tamanho_dados, fp);

        if (!recebido[pacote.numero_pacote]) {
            recebido[pacote.numero_pacote] = 1;
            total_recebidos++;
            if (pacote.numero_pacote > *maior_pacote)
                *maior_pacote = pacote.numero_pacote;
        }
    }

    return total_recebidos;
}

void calcularEMostrarEstatisticas(struct timespec inicio, struct timespec fim, int maior_pacote, int total_recebidos, const char *filepath) {
    long long tempo_ns = (fim.tv_sec - inicio.tv_sec) * 1000000000LL + (fim.tv_nsec - inicio.tv_nsec); // Correção para LL e cálculo direto
    double tempo_s = (double)tempo_ns / 1.0e9;
    long long tempo_ms = tempo_ns / 1000000LL;

    int total_esperado = maior_pacote + 1;
    int perdidos = total_esperado - total_recebidos;
    double taxa_kBps = 0.0;
    if (tempo_s > 0) { // Evitar divisão por zero se o tempo for muito pequeno
        taxa_kBps = (double)total_recebidos * (BUFFER_SIZE - 8) / tempo_s / 1024.0;
    }


    printf("Total esperado: %d pacotes\n", total_esperado);
    printf("Total recebido: %d pacotes\n", total_recebidos);
    printf("Perdidos: %d pacotes\n", perdidos);
    printf("Taxa de download: %.2f KB/s\n", taxa_kBps);
    printf("Tempo total: %lld ns (%lld ms, %.2fs)\n", tempo_ns, tempo_ms, tempo_s); // Adicionado %lld ns

    // Calcular hash
    unsigned char hashCalculado[EVP_MAX_MD_SIZE];
    calcularHashMd5(filepath, hashCalculado);
    salvarHashMd5(HASH_FILENAME, hashCalculado);

    if (perdidos == 0) {
        if (conferirHashMd5(HASH_FILENAME, filepath)) {
            printf("Integridade verificada: o arquivo está correto.\n");
        } else {
            printf("Integridade falhou: o arquivo está corrompido.\n");
        }
    } else {
        printf("Integridade não verificada: houve perdas.\n");
    }
}
