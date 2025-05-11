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
#include <libgen.h>
#include <openssl/md5.h>

#include "hashMd5.h"

#define BUFFER_SIZE 4096
#define NOME_DIRETORIO "Recebidos"
#define ARQUIVO_HASH "hashRecebido.txt"
#define MAX_FILENAME_LEN 256
#define SAVE_FILENAME_PREFIX "recebido_"
#define MAX_PATH_LEN (MAX_FILENAME_LEN + sizeof(NOME_DIRETORIO) + sizeof(SAVE_FILENAME_PREFIX) + 2)

// Assinaturas
void criarDiretorioSeNecessario();
void construirCaminhos(char *base_name, char *save_filepath, char *hash_filepath);
int conectarAoServidor(struct sockaddr_in *server_addr, const char *ip, int porta);
void enviarNomeArquivo(int sock, const char *filename);
void receberHash(int sock, const char *hash_filepath, unsigned char *hash_buffer);
void receberArquivo(int sock, const char *save_filepath, size_t *total_bytes);
void calcularVelocidade(struct timespec inicio, struct timespec fim, size_t total_bytes);
void verificarHash(const char *hash_filepath, const char *save_filepath);

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Uso: %s <IP_SERVIDOR> <PORTA> <NOME_ARQUIVO>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *ip = argv[1];
    int porta = atoi(argv[2]);
    const char *nome_arquivo = argv[3];

    char *base_name = basename((char *)nome_arquivo);
    char save_filepath[MAX_PATH_LEN], hash_filepath[MAX_PATH_LEN];
    unsigned char hash_buffer[MD5_DIGEST_LENGTH];
    struct sockaddr_in server_addr;
    int sock;
    size_t total_bytes = 0;
    struct timespec inicio, fim;

    criarDiretorioSeNecessario();
    construirCaminhos(base_name, save_filepath, hash_filepath);

    sock = conectarAoServidor(&server_addr, ip, porta);
    clock_gettime(CLOCK_MONOTONIC, &inicio);

    enviarNomeArquivo(sock, nome_arquivo);
    receberHash(sock, hash_filepath, hash_buffer);
    receberArquivo(sock, save_filepath, &total_bytes);

    clock_gettime(CLOCK_MONOTONIC, &fim);
    close(sock);

    calcularVelocidade(inicio, fim, total_bytes);
    verificarHash(hash_filepath, save_filepath);

    return 0;
}

void criarDiretorioSeNecessario() {
    struct stat st = {0};
    if (stat(NOME_DIRETORIO, &st) == -1) {
        if (mkdir(NOME_DIRETORIO, 0700) == -1 && errno != EEXIST) {
            perror("Erro ao criar diretório");
            exit(EXIT_FAILURE);
        }
    }
}

void construirCaminhos(char *base_name, char *save_filepath, char *hash_filepath) {
    snprintf(save_filepath, MAX_PATH_LEN, "%s/%s%s", NOME_DIRETORIO, SAVE_FILENAME_PREFIX, base_name);
    snprintf(hash_filepath, MAX_PATH_LEN, "%s/%s", NOME_DIRETORIO, ARQUIVO_HASH);
}

int conectarAoServidor(struct sockaddr_in *server_addr, const char *ip, int porta) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Erro ao criar socket");
        exit(EXIT_FAILURE);
    }

    server_addr->sin_family = AF_INET;
    server_addr->sin_port = htons(porta);
    if (inet_pton(AF_INET, ip, &server_addr->sin_addr) <= 0) {
        perror("Endereço IP inválido");
        close(sock);
        exit(EXIT_FAILURE);
    }

    if (connect(sock, (struct sockaddr *)server_addr, sizeof(*server_addr)) < 0) {
        perror("Erro ao conectar ao servidor");
        close(sock);
        exit(EXIT_FAILURE);
    }

    printf("Conectado ao servidor %s:%d\n", ip, porta);
    return sock;
}

void enviarNomeArquivo(int sock, const char *filename) {
    if (send(sock, filename, strlen(filename), 0) < 0) {
        perror("Erro ao enviar nome do arquivo");
        close(sock);
        exit(EXIT_FAILURE);
    }
    printf("Solicitação do arquivo '%s' enviada.\n", filename);
}

void receberHash(int sock, const char *hash_filepath, unsigned char *hash_buffer) {
    FILE *fp = fopen(hash_filepath, "wb");
    if (!fp) {
        perror("Erro ao criar arquivo de hash");
        close(sock);
        exit(EXIT_FAILURE);
    }

    size_t total = 0;
    ssize_t r;
    while (total < MD5_DIGEST_LENGTH) {
        r = recv(sock, hash_buffer + total, MD5_DIGEST_LENGTH - total, 0);
        if (r <= 0) {
            perror("Erro ao receber hash");
            fclose(fp);
            remove(hash_filepath);
            close(sock);
            exit(EXIT_FAILURE);
        }
        total += r;
    }

    fwrite(hash_buffer, 1, MD5_DIGEST_LENGTH, fp);
    fclose(fp);
    printf("Hash MD5 recebido e salvo em '%s'.\n", hash_filepath);
}

void receberArquivo(int sock, const char *save_filepath, size_t *total_bytes) {
    FILE *fp = fopen(save_filepath, "wb");
    if (!fp) {
        perror("Erro ao criar arquivo local");
        close(sock);
        exit(EXIT_FAILURE);
    }

    char buffer[BUFFER_SIZE];
    ssize_t r;

    while ((r = recv(sock, buffer, BUFFER_SIZE, 0)) > 0) {
        if (fwrite(buffer, 1, r, fp) < r) {
            perror("Erro ao salvar dados no arquivo");
            fclose(fp);
            remove(save_filepath);
            close(sock);
            exit(EXIT_FAILURE);
        }
        *total_bytes += r;
    }

    if (r < 0) {
        perror("Erro na recepção do arquivo");
        fclose(fp);
        remove(save_filepath);
        close(sock);
        exit(EXIT_FAILURE);
    }

    fclose(fp);
    printf("Arquivo '%s' recebido com sucesso (%zu bytes).\n", save_filepath, *total_bytes);
}

void calcularVelocidade(struct timespec inicio, struct timespec fim, size_t total_bytes) {
    long long t_ns = (fim.tv_sec - inicio.tv_sec) * 1000000000LL + (fim.tv_nsec - inicio.tv_nsec);
    printf("Tempo total: %lld ns\n", t_ns);

    if (t_ns > 0 && total_bytes > 0) {
        double t_s = (double)t_ns / 1.0e9;
        double kBps = total_bytes / 1024.0 / t_s;
        double MBps = total_bytes / (1024.0 * 1024.0) / t_s;
        printf("Velocidade de download: %.2f KB/s (%.2f MB/s)\n", kBps, MBps);
    } else {
        printf("Impossível calcular a velocidade.\n");
    }
}

void verificarHash(const char *hash_filepath, const char *save_filepath) {
    int ok = conferirHashMd5(hash_filepath, save_filepath);
    if (ok)
        printf("Hash MD5 conferido: OK\n\n");
    else
        printf("Hash MD5 conferido: ERRO\n\n");
}
