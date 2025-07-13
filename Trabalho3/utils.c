#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

bool checkSum(int esperado, int atual) {
    return (esperado == atual) ? true : false;
}

void logger(const char* msg, const char* filename) {
    if (msg != NULL && filename != NULL) {
        FILE *file = fopen(filename, "a");
        if (file != NULL) {
            char timestamp[26];
            obter_timestamp_string(timestamp, sizeof(timestamp));
            fprintf(file, "[%s] %s\n", timestamp, msg);
            fclose(file);
        }
    } else {
        fprintf(stderr, "Erro: Mensagem NULL passada para logger.\n");
    }
}

void obter_timestamp_string(char *buffer, int tamanho_buffer) {
    time_t agora;
    struct tm *info_tempo;
    
    time(&agora);
    info_tempo = localtime(&agora);
    strftime(buffer, tamanho_buffer, "%Y-%m-%d %H:%M:%S", info_tempo);
}


void log_com_timestamp(const char *mensagem, int modo_verboso) {
    // Exibe no console apenas se verbose 
    if (modo_verboso && mensagem != NULL) {
        char timestamp[26];
        obter_timestamp_string(timestamp, sizeof(timestamp));
        printf("[%s] %s\n", timestamp, mensagem);
    }
}

uint32_t calcular_checksum(const char *dados, int tamanho) {
    uint32_t soma = 0;
    for (int i = 0; i < tamanho; i++) {
        soma += (unsigned char)dados[i];
    }
    return soma;
}

int verificar_integridade_pacote(const void *pacote, uint32_t checksum_esperado, int tamanho_dados) {
    if (tamanho_dados <= 0) return 1; // Pacote sem dados é válido
    
    const char *dados = (const char*)pacote;
    uint32_t checksum_calculado = calcular_checksum(dados, tamanho_dados);
    return checksum_calculado == checksum_esperado;
}

int simular_perda_pacote(double taxa_perda) {
    if (taxa_perda <= 0.0) return 0;
    if (taxa_perda >= 1.0) return 1;
    
    double valor_aleatorio = (double)rand() / RAND_MAX;
    return valor_aleatorio < taxa_perda;
}
void imprimir_estatisticas_servidor(uint32_t pacotes_recebidos, uint32_t pacotes_enviados, 
                                   uint32_t acks_enviados, uint32_t nacks_enviados,
                                   uint32_t pacotes_corrompidos, uint32_t pacotes_duplicados,
                                   time_t tempo_inicio, time_t tempo_fim) {
    char buffer[256];
    
    printf("\n=== ESTATÍSTICAS DO SERVIDOR ===\n");
    logger("=== ESTATÍSTICAS DO SERVIDOR ===", "server.log");
    
    printf("Pacotes recebidos: %u\n", pacotes_recebidos);
    sprintf(buffer, "Pacotes recebidos: %u", pacotes_recebidos);
    logger(buffer, "server.log");
    
    printf("Pacotes enviados: %u\n", pacotes_enviados);
    sprintf(buffer, "Pacotes enviados: %u", pacotes_enviados);
    logger(buffer, "server.log");
    
    printf("ACKs enviados: %u\n", acks_enviados);
    sprintf(buffer, "ACKs enviados: %u", acks_enviados);
    logger(buffer, "server.log");
    
    printf("NACKs enviados: %u\n", nacks_enviados);
    sprintf(buffer, "NACKs enviados: %u", nacks_enviados);
    logger(buffer, "server.log");
    
    printf("Pacotes corrompidos: %u\n", pacotes_corrompidos);
    sprintf(buffer, "Pacotes corrompidos: %u", pacotes_corrompidos);
    logger(buffer, "server.log");
    
    printf("Pacotes duplicados: %u\n", pacotes_duplicados);
    sprintf(buffer, "Pacotes duplicados: %u", pacotes_duplicados);
    logger(buffer, "server.log");
    
    double duracao = difftime(tempo_fim, tempo_inicio);
    printf("Tempo total: %.2f segundos\n", duracao);
    sprintf(buffer, "Tempo total: %.2f segundos", duracao);
    logger(buffer, "server.log");
    
    if (pacotes_recebidos > 0) {
        double taxa_sucesso = ((double)(pacotes_recebidos - pacotes_corrompidos) / pacotes_recebidos) * 100;
        printf("Taxa de sucesso: %.2f%%\n", taxa_sucesso);
        sprintf(buffer, "Taxa de sucesso: %.2f%%", taxa_sucesso);
        logger(buffer, "server.log");
    }
    printf("================================\n");
    logger("================================", "server.log");
}

void imprimir_estatisticas_cliente(uint32_t pacotes_enviados, uint32_t pacotes_recebidos,
                                  uint32_t retransmissoes, uint32_t timeouts,
                                  uint32_t total_bytes_enviados, time_t tempo_inicio, time_t tempo_fim) {
    char buffer[256];
    
    printf("\n=== ESTATÍSTICAS DO CLIENTE ===\n");
    logger("=== ESTATÍSTICAS DO CLIENTE ===", "cliente.log");
    
    printf("Pacotes enviados: %u\n", pacotes_enviados);
    sprintf(buffer, "Pacotes enviados: %u", pacotes_enviados);
    logger(buffer, "cliente.log");
    
    printf("Pacotes recebidos (ACKs): %u\n", pacotes_recebidos);
    sprintf(buffer, "Pacotes recebidos (ACKs): %u", pacotes_recebidos);
    logger(buffer, "cliente.log");
    
    printf("Retransmissões: %u\n", retransmissoes);
    sprintf(buffer, "Retransmissões: %u", retransmissoes);
    logger(buffer, "cliente.log");
    
    printf("Timeouts: %u\n", timeouts);
    sprintf(buffer, "Timeouts: %u", timeouts);
    logger(buffer, "cliente.log");
    
    printf("Total de bytes enviados: %u\n", total_bytes_enviados);
    sprintf(buffer, "Total de bytes enviados: %u", total_bytes_enviados);
    logger(buffer, "cliente.log");
    
    double duracao = difftime(tempo_fim, tempo_inicio);
    printf("Tempo total: %.2f segundos\n", duracao);
    sprintf(buffer, "Tempo total: %.2f segundos", duracao);
    logger(buffer, "cliente.log");
    
    if (pacotes_enviados > 0) {
        double taxa_sucesso = ((double)pacotes_recebidos / pacotes_enviados) * 100;
        printf("Taxa de sucesso: %.2f%%\n", taxa_sucesso);
        sprintf(buffer, "Taxa de sucesso: %.2f%%", taxa_sucesso);
        logger(buffer, "cliente.log");
        
        if (duracao > 0) {
            double throughput = total_bytes_enviados / duracao;
            printf("Throughput: %.2f bytes/s\n", throughput);
            sprintf(buffer, "Throughput: %.2f bytes/s", throughput);
            logger(buffer, "cliente.log");
        }
    }
    printf("===============================\n");
    logger("===============================", "cliente.log");
}
