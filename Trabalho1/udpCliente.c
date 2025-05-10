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

int main(int argc, char *argv[]) {
    int sock;
    struct sockaddr_in server_addr;
    socklen_t addr_len = sizeof(server_addr);
    Pacote pacote;
    FILE *fp;
    int recebido[MAX_PACKETS] = {0};
    int total_recebidos = 0, maior_pacote = 0;
    ssize_t bytes_recebidos;
    struct timespec inicio, fim;

    if (argc != 2) {
        fprintf(stderr, "Uso: %s <nome_arquivo>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Criar diretório
    struct stat st = {0};
    if (stat(RECEIVED_DIR, &st) == -1) {
        mkdir(RECEIVED_DIR, 0700);
    }

    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s/%s", RECEIVED_DIR, SAVE_FILENAME);
    fp = fopen(filepath, "wb");
    if (!fp) {
        perror("Erro ao criar arquivo");
        exit(EXIT_FAILURE);
    }

    // Criar socket
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("Erro ao criar socket");
        exit(EXIT_FAILURE);
    }

    // Endereço do servidor
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    // Enviar solicitação de arquivo
    sendto(sock, argv[1], strlen(argv[1]), 0,
           (struct sockaddr *)&server_addr, sizeof(server_addr));

    // Marcar início do tempo
    clock_gettime(CLOCK_MONOTONIC, &inicio);

    // Receber pacotes
    while (1) {
        bytes_recebidos = recvfrom(sock, &pacote, sizeof(pacote), 0,
                                   (struct sockaddr *)&server_addr, &addr_len);
        if (bytes_recebidos < 0) break;

        if (pacote.tamanho_dados == 0) break; // Pacote de fim

        if (pacote.numero_pacote >= MAX_PACKETS) continue;

        fseek(fp, pacote.numero_pacote * (BUFFER_SIZE - 8), SEEK_SET);
        fwrite(pacote.dados, 1, pacote.tamanho_dados, fp);
        if (!recebido[pacote.numero_pacote]) {
            recebido[pacote.numero_pacote] = 1;
            total_recebidos++;
            if (pacote.numero_pacote > maior_pacote)
                maior_pacote = pacote.numero_pacote;
        }
    }

    // Marcar fim do tempo
    clock_gettime(CLOCK_MONOTONIC, &fim);
    fclose(fp);
    close(sock);

    long long tempo_ns = (fim.tv_sec - inicio.tv_sec) * 1e9 + (fim.tv_nsec - inicio.tv_nsec);
    double tempo_s = tempo_ns / 1e9;
    long long tempo_ms = tempo_ns / 1000000;


    int total_esperado = maior_pacote + 1;
    int perdidos = total_esperado - total_recebidos;
    double taxa = (double)total_recebidos * (BUFFER_SIZE - 8) / tempo_s / 1024;

    printf("Total esperado: %d pacotes\n", total_esperado);
    printf("Total recebido: %d pacotes\n", total_recebidos);
    printf("Perdidos: %d pacotes\n", perdidos);
    printf("Taxa de download: %.2f KB/s\n", taxa);
    printf("Tempo total: %lld ms (%.2fs)\n", tempo_ms, tempo_s);

	// Calcular o hash MD5 do arquivo recebido
	unsigned char hashCalculado[EVP_MAX_MD_SIZE];
	calcularHashMd5(filepath, hashCalculado);

	// Salvar o hash calculado para posterior comparação
	salvarHashMd5(HASH_FILENAME, hashCalculado);

	// Conferir a integridade do arquivo recebendo o hash
	if (perdidos == 0) {
		if (conferirHashMd5(HASH_FILENAME, filepath)) {
			printf("Integridade verificada: o arquivo está correto.\n");
		} else {
			printf("Integridade falhou: o arquivo está corrompido.\n");
		}
	} else {
		printf("Integridade não verificada: houve perdas.\n");
	}

    return 0;
}
