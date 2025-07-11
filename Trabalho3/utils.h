#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>


bool checkSum(int esperado, int atual);
void logger(const char* msg, const char* filename);
uint32_t calcular_checksum(const char *dados, int tamanho);
int verificar_integridade_pacote(const void *pacote, uint32_t checksum_esperado, int tamanho_dados);
int simular_perda_pacote(double taxa_perda);
void log_com_timestamp(const char *mensagem, int modo_verboso);
void imprimir_estatisticas_servidor(uint32_t pacotes_recebidos, uint32_t pacotes_enviados, 
                                    uint32_t acks_enviados, uint32_t nacks_enviados,
                                    uint32_t pacotes_corrompidos, uint32_t pacotes_duplicados,
                                    time_t tempo_inicio, time_t tempo_fim);
void imprimir_estatisticas_cliente(uint32_t pacotes_enviados, uint32_t pacotes_recebidos,
                                    uint32_t retransmissoes, uint32_t timeouts,
                                    uint32_t total_bytes_enviados, time_t tempo_inicio, time_t tempo_fim);
void obter_timestamp_string(char *buffer, int tamanho_buffer);

#endif 