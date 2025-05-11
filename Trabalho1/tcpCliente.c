#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <time.h>
#include <openssl/md5.h> // Para MD5_DIGEST_LENGTH
#include <sys/stat.h>   // Para mkdir
#include <sys/types.h>  // Para mkdir
#include <errno.h>      // Para errno com mkdir
#include <errno.h>      // Para errno com mkdir
#include <libgen.h>     // Para basename()

#include "hashMd5.h" // calcularHashMd5, salvarHashMd5, conferirHashMd5

#define _POSIX_C_SOURCE 200809L
#define SERVER_IP "127.0.0.1" // IP do servidor (localhost neste exemplo)
#define SERVER_PORT 8081      // Porta do servidor
#define BUFFER_SIZE 4096      // Tamanho do bloco (4KB)
#define RECEIVED_DIR "Recebidos" // Nome do diretório para salvar os arquivos
#define HASH_FILENAME "hashRecebido.txt" // Nome base para salvar o hash MD5 recebido
#define MAX_FILENAME_LEN 256 // Tamanho máximo para nomes de arquivo
#define SAVE_FILENAME_PREFIX "recebido_" // Prefixo para o nome do arquivo salvo
#define MAX_PATH_LEN (MAX_FILENAME_LEN + sizeof(RECEIVED_DIR) + sizeof(SAVE_FILENAME_PREFIX) + 2) // +1 para '/' e +1 para '\0'


int main(int argc, char *argv[]) {
    int sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    char filename_request[MAX_FILENAME_LEN]; // Buffer para o nome do arquivo solicitado
    char save_filename_base[MAX_FILENAME_LEN + sizeof(SAVE_FILENAME_PREFIX)]; // Nome base do arquivo salvo
    char save_filepath[MAX_PATH_LEN]; // Caminho completo para salvar o arquivo
    char hash_filepath[MAX_PATH_LEN]; // Caminho completo para salvar o hash


    unsigned char hash_buffer[MD5_DIGEST_LENGTH]; // Buffer para receber o hash
    FILE *fp_save, *fp_hash; // Ponteiros separados

    ssize_t bytes_received;
    size_t total_hash_bytes_received = 0;

    struct timespec tempoInicial_ts, tempoFinal_ts;

    if (argc != 2) {
        fprintf(stderr, "Uso: %s <nome_arquivo>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    // Nome do arquivo a solicitar
    strncpy(filename_request, argv[1], MAX_FILENAME_LEN - 1);
    filename_request[MAX_FILENAME_LEN - 1] = '\0'; // Garantir terminação nula

    // --- Criar diretório de recebidos se não existir ---
    struct stat st = {0};
    if (stat(RECEIVED_DIR, &st) == -1) {
        if (mkdir(RECEIVED_DIR, 0700) == -1 && errno != EEXIST) { // 0700: permissões rwx para o usuário
             perror("Erro ao criar diretório 'Recebidos'");
             exit(EXIT_FAILURE);
        }
        printf("Diretório '%s' criado.\n", RECEIVED_DIR);
    }


    // --- Extrair o nome base do arquivo solicitado ---
   char *base_name = basename(argv[1]); // Extrai "100b.txt" de "Arquivos/100b.txt"

    // Criar nome base do arquivo para salvar localmente usando o nome base extraído
    snprintf(save_filename_base, sizeof(save_filename_base), "%s%s", SAVE_FILENAME_PREFIX, base_name);
    // Criar caminho completo para salvar o arquivo
    snprintf(save_filepath, sizeof(save_filepath), "%s/%s", RECEIVED_DIR, save_filename_base);
    // Criar caminho completo para salvar o hash
    snprintf(hash_filepath, sizeof(hash_filepath), "%s/%s", RECEIVED_DIR, HASH_FILENAME);


    // Criar socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Erro ao criar socket");
        exit(EXIT_FAILURE);
    }

    // Configurar endereço do servidor
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    // Conectar ao servidor
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Erro ao conectar ao servidor");
        close(sock);
        exit(EXIT_FAILURE);
    }
    printf("Conectado ao servidor %s:%d\n", SERVER_IP, SERVER_PORT);


    if(clock_gettime(CLOCK_MONOTONIC, &tempoInicial_ts) == -1) {
        perror("Erro ao obter tempo inicial");
    }



    // Enviar nome do arquivo solicitado ao servidor
    if (send(sock, filename_request, strlen(filename_request), 0) < 0) {
        perror("Erro ao enviar nome do arquivo");
        close(sock);
        exit(EXIT_FAILURE);
    }
    printf("Solicitação do arquivo '%s' enviada.\n", filename_request);

    // --- Etapa 1: Receber o hash MD5 primeiro ---
    printf("Recebendo hash MD5...\n");
    fp_hash = fopen(hash_filepath, "wb"); // Usa o caminho completo
    if (fp_hash == NULL) {
        perror("Erro ao criar arquivo local para hash");
        close(sock);
        exit(EXIT_FAILURE);
    }

    // Ler exatamente MD5_DIGEST_LENGTH bytes para o hash
    while (total_hash_bytes_received < MD5_DIGEST_LENGTH) {
        bytes_received = recv(sock, hash_buffer + total_hash_bytes_received,
                              MD5_DIGEST_LENGTH - total_hash_bytes_received, 0);
        if (bytes_received <= 0) {
            perror("Erro ao receber hash ou conexão fechada prematuramente");
            fclose(fp_hash);
            remove(hash_filepath); // Usa o caminho completo
            close(sock);
            exit(EXIT_FAILURE);
        }
        total_hash_bytes_received += bytes_received;
    }

    // Escrever o hash recebido no arquivo
    if (fwrite(hash_buffer, 1, MD5_DIGEST_LENGTH, fp_hash) < MD5_DIGEST_LENGTH) {
        perror("Erro ao escrever hash no arquivo local");
        fclose(fp_hash);
        remove(hash_filepath); // Usa o caminho completo
        close(sock);
        exit(EXIT_FAILURE);
    }
    fclose(fp_hash); // Fecha o arquivo de hash
    printf("Hash MD5 recebido e salvo em '%s'.\n", hash_filepath);


    // --- Etapa 2: Receber o arquivo principal ---
    // Abrir arquivo local para escrita do arquivo principal
    fp_save = fopen(save_filepath, "wb"); // Usa o caminho completo
    if (fp_save == NULL) {
        perror("Erro ao criar arquivo local para dados");
        // O hash já foi salvo, mas o arquivo principal falhou
        close(sock);
        exit(EXIT_FAILURE);
    }

    // Receber dados do servidor em blocos e escrever no arquivo
    printf("Recebendo arquivo principal...\n");
    memset(buffer, 0, BUFFER_SIZE); // Limpar buffer principal
    size_t total_file_bytes_received = 0; // Para acumular o tamanho do arquivo principal
    while ((bytes_received = recv(sock, buffer, BUFFER_SIZE, 0)) > 0) {
        // Escrever bloco recebido no arquivo
        if (fwrite(buffer, 1, bytes_received, fp_save) < bytes_received) {
            perror("Erro ao escrever dados no arquivo local");
            fclose(fp_save);
            remove(save_filepath); // Usa o caminho completo
            close(sock);
            exit(EXIT_FAILURE);
        }
        total_file_bytes_received += bytes_received; // Acumula os bytes do arquivo
    }

    if (bytes_received < 0) {
        perror("Erro ao receber dados do arquivo");
        // Arquivo pode estar incompleto
        fclose(fp_save);
        remove(save_filepath); // Usa o caminho completo
        close(sock); // Adicionado para fechar o socket em caso de erro
        exit(EXIT_FAILURE);
    } else {
        printf("Arquivo '%s' recebido com sucesso (%zu bytes).\n", save_filepath, total_file_bytes_received);
    }
    fclose(fp_save); // Fecha o arquivo principal

    //calcula tempo total de execução
    if (clock_gettime(CLOCK_MONOTONIC, &tempoFinal_ts) == -1) {
        perror("Erro ao obter tempo final");
        // Considerar tratamento de erro, como não calcular a velocidade
    }
        // Calcular a diferença de tempo em milissegundos
    long long diff_sec = tempoFinal_ts.tv_sec - tempoInicial_ts.tv_sec;
    long long diff_nsec = tempoFinal_ts.tv_nsec - tempoInicial_ts.tv_nsec;

    long long tempoTotal_ns =  (diff_sec * 1000000000LL) + diff_nsec;

    printf("Tempo total para obter o arquivo (incluindo hash): %lld ns\n", tempoTotal_ns);

    // --- Calcular e imprimir velocidade de download ---
    if (total_file_bytes_received > 0 && tempoTotal_ns > 0) {
        double tempo_s = (double)tempoTotal_ns / 1.0e9; // Tempo total em segundos
        // Velocidade = (Total de Bytes / 1024 para KB) / Tempo em Segundos
        double velocidade_kBps = (double)total_file_bytes_received / 1024.0 / tempo_s;
        // Velocidade = (Total de Bytes / (1024*1024) para MB) / Tempo em Segundos
        double velocidade_MBps = (double)total_file_bytes_received / (1024.0 * 1024.0) / tempo_s;

        printf("Velocidade de download: %.2f KB/s (%.2f MB/s)\n", velocidade_kBps, velocidade_MBps);
    } else if (total_file_bytes_received == 0) {
        printf("Nenhum dado de arquivo recebido, não é possível calcular a velocidade.\n");
    } else { // tempoTotal_ns <= 0
        printf("Tempo de download inválido ou zero, não é possível calcular a velocidade.\n");
    }
    // --- Etapa 3: Conferir hash MD5 do arquivo recebido ---
    printf("Conferindo hash MD5...\n");
    // Usa os caminhos completos para conferir o hash
    int resultadoHash = conferirHashMd5(hash_filepath, save_filepath);

    if (resultadoHash == 1) {
        printf("Hash MD5 conferido: OK\n");
    } else {
        printf("Hash MD5 conferido: ERRO\n");
    }

    // --- Etapa 4: fechando Socket---
    printf("Fechando socket...\n");

 
    close(sock);

    return 0;
}
