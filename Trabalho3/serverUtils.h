#ifndef SERVER_UTILS_H
#define SERVER_UTILS_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include "utils.h"

#define MAX_BUFFER_SIZE 1024
#define DEFAULT_PORT 8080

// Tipos de mensagem
typedef enum {
    MSG_START_TRANSMISSION = 1,
    MSG_DATA = 2,
    MSG_END_TRANSMISSION = 3,
    MSG_ACK = 4,
    MSG_NACK = 5
} MessageType;

// Estrutura do pacote com checksum
typedef struct {
    MessageType tipo;
    uint32_t num_sequencia;
    uint32_t tamanho_dados;
    uint32_t checksum;
    char dados[MAX_BUFFER_SIZE - sizeof(MessageType) - sizeof(uint32_t) * 3];
} Pacote;

// Estrutura para estatísticas do servidor
typedef struct {
    uint32_t pacotes_recebidos;
    uint32_t pacotes_enviados;
    uint32_t acks_enviados;
    uint32_t nacks_enviados;
    uint32_t pacotes_corrompidos;
    uint32_t pacotes_duplicados;
    time_t tempo_inicio;
    time_t tempo_fim;
} EstatisticasServidor;

typedef struct {
    int socket_fd;
    struct sockaddr_in endereco_servidor;
    struct sockaddr_in endereco_cliente;
    socklen_t tamanho_cliente;
    int porta;
    int esta_aberto;
    uint32_t sequencia_esperada;
    int modo_verboso;
    double taxa_perda;
    EstatisticasServidor estatisticas;
} ServidorUDP;

// Funções básicas do servidor UDP
int servidor_udp_abrir(ServidorUDP *servidor, int porta);
int servidor_udp_fechar(ServidorUDP *servidor);
int servidor_udp_receber_pacote(ServidorUDP *servidor, Pacote *pacote);
int servidor_udp_enviar_ack(ServidorUDP *servidor, uint32_t num_seq);
int servidor_udp_enviar_nack(ServidorUDP *servidor, uint32_t num_seq);
void servidor_udp_inicializar(ServidorUDP *servidor);
void servidor_udp_imprimir_info(ServidorUDP *servidor);
int servidor_udp_processar_transmissao(ServidorUDP *servidor);
void servidor_udp_definir_verboso(ServidorUDP *servidor, int verboso);
void servidor_udp_definir_taxa_perda(ServidorUDP *servidor, double taxa_perda);

#endif // SERVER_UTILS_H