#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <time.h>
#include <openssl/md5.h> // Para MD5_DIGEST_LENGTH

#include "hashMd5.h" // calcularHashMd5, salvarHashMd5, conferirHashMd5

#define _POSIX_C_SOURCE 200809L
#define SERVER_IP "127.0.0.1" // IP do servidor (localhost neste exemplo)
#define SERVER_PORT 8081      // Porta do servidor
#define BUFFER_SIZE 4096      // Tamanho do bloco (4KB)
#define FILENAME_REQUEST "teste.txt" // Nome do arquivo a solicitar
#define SAVE_FILENAME "testeRecebido.txt"     // Nome para salvar o arquivo recebido
#define HASH_RECEIVED_FILENAME "hashRecebido.txt" // Nome para salvar o hash MD5 recebido


int main() {
    int sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    unsigned char hash_buffer[MD5_DIGEST_LENGTH]; // Buffer para receber o hash
    FILE *fp_save, *fp_hash; // Ponteiros separados
    
    ssize_t bytes_received;
    size_t total_hash_bytes_received = 0;
  
    struct timespec tempoInicial_ts, tempoFinal_ts;

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
    if (send(sock, FILENAME_REQUEST, strlen(FILENAME_REQUEST), 0) < 0) {
        perror("Erro ao enviar nome do arquivo");
        close(sock);
        exit(EXIT_FAILURE);
    }
    printf("Solicitação do arquivo '%s' enviada.\n", FILENAME_REQUEST);

    // --- Etapa 1: Receber o hash MD5 primeiro ---
    printf("Recebendo hash MD5...\n");
    fp_hash = fopen(HASH_RECEIVED_FILENAME, "wb");
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
            remove(HASH_RECEIVED_FILENAME); // Remove arquivo parcial
            close(sock);
            exit(EXIT_FAILURE);
        }
        total_hash_bytes_received += bytes_received;
    }

    // Escrever o hash recebido no arquivo
    if (fwrite(hash_buffer, 1, MD5_DIGEST_LENGTH, fp_hash) < MD5_DIGEST_LENGTH) {
        perror("Erro ao escrever hash no arquivo local");
        fclose(fp_hash);
        remove(HASH_RECEIVED_FILENAME);
        close(sock);
        exit(EXIT_FAILURE);
    }
    fclose(fp_hash); // Fecha o arquivo de hash
    printf("Hash MD5 recebido e salvo em '%s'.\n", HASH_RECEIVED_FILENAME);


    // --- Etapa 2: Receber o arquivo principal ---
    // Abrir arquivo local para escrita do arquivo principal
    fp_save = fopen(SAVE_FILENAME, "wb"); // "wb" para escrita binária
    if (fp_save == NULL) {
        perror("Erro ao criar arquivo local para dados");
        // O hash já foi salvo, mas o arquivo principal falhou
        close(sock);
        exit(EXIT_FAILURE);
    }

    // Receber dados do servidor em blocos e escrever no arquivo
    printf("Recebendo arquivo principal...\n");
    memset(buffer, 0, BUFFER_SIZE); // Limpar buffer principal
    while ((bytes_received = recv(sock, buffer, BUFFER_SIZE, 0)) > 0) {
        // Escrever bloco recebido no arquivo
        if (fwrite(buffer, 1, bytes_received, fp_save) < bytes_received) {
             perror("Erro ao escrever dados no arquivo local");
             fclose(fp_save);
             remove(SAVE_FILENAME); // Remove arquivo parcial
             close(sock);
             exit(EXIT_FAILURE);
        }
         // memset(buffer, 0, BUFFER_SIZE); // Limpar para próxima leitura (opcional mas boa prática)
    }

    if (bytes_received < 0) {
        perror("Erro ao receber dados do arquivo");
        // Arquivo pode estar incompleto
        fclose(fp_save);
        remove(SAVE_FILENAME);
        close(sock);
        exit(EXIT_FAILURE);
    } else {
        printf("Arquivo '%s' recebido com sucesso.\n", SAVE_FILENAME);
    }
    fclose(fp_save); // Fecha o arquivo principal


    // --- Etapa 3: Conferir hash MD5 do arquivo recebido ---
    printf("Conferindo hash MD5...\n");
    // Usa o hash que acabamos de salvar (HASH_RECEIVED_FILENAME)
    // e o arquivo que acabamos de salvar (SAVE_FILENAME)
    int resultadoHash = conferirHashMd5(HASH_RECEIVED_FILENAME, SAVE_FILENAME);

    if (resultadoHash == 1) {
        printf("Hash MD5 conferido: OK\n");
    } else {
        printf("Hash MD5 conferido: ERRO\n");
    }

    // --- Etapa 4: Limpeza e Tempo ---
    printf("Fechando socket...\n");

    if (clock_gettime(CLOCK_MONOTONIC, &tempoFinal_ts) == -1) {
        perror("Erro ao obter tempo final");
        // Continuar mesmo assim?
   }
       // Calcular a diferença de tempo em milissegundos
    long long diff_sec = tempoFinal_ts.tv_sec - tempoInicial_ts.tv_sec;
    long long diff_nsec = tempoFinal_ts.tv_nsec - tempoInicial_ts.tv_nsec;

    long long tempoTotal_ns =  (diff_sec * 1000000000LL) + diff_nsec;
   
    printf("Tempo total de execução: %lld ns\n", tempoTotal_ns);
    close(sock);

    return 0;
}