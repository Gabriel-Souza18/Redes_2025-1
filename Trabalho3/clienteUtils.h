#ifndef CLIENT_UTILS_H
#define CLIENT_UTILS_H

#include <netinet/in.h>
#include <time.h>
#include <stdint.h>
#include "utils.h"

#define DEFAULT_PORT 8080
#define BUFFER_SIZE 1024
#define TIMEOUT_SEC 5
#define MAX_RETRIES 3

// Tipos de mensagem
typedef enum {
    MSG_START_TRANSMISSION = 1,
    MSG_DATA = 2,
    MSG_END_TRANSMISSION = 3,
    MSG_ACK = 4,
    MSG_NACK = 5
} MessageType;

// pacote
typedef struct {
    MessageType tipo;
    uint32_t num_sequencia;
    uint32_t tamanho_dados;
    uint32_t checksum;
    char dados[BUFFER_SIZE - sizeof(MessageType) - sizeof(uint32_t) * 3];
} Pacote;

// estatísticas do cliente
typedef struct {
    uint32_t pacotes_enviados;
    uint32_t pacotes_recebidos;
    uint32_t retransmissoes;
    uint32_t timeouts;
    uint32_t total_bytes_enviados;
    time_t tempo_inicio;
    time_t tempo_fim;
} EstatisticasCliente;

// cliente UDP
typedef struct {
    int socket_fd;
    struct sockaddr_in endereco_servidor;
    uint32_t num_sequencia;
    int timeout_seg;
    int modo_verboso;
    EstatisticasCliente estatisticas;
} ClienteUDP;

void cliente_udp_inicializar(ClienteUDP *cliente, const char *ip_servidor, int porta);
int cliente_udp_enviar_pacote(ClienteUDP *cliente, const Pacote *pacote);
int cliente_udp_receber_ack(ClienteUDP *cliente, uint32_t seq_esperada);
int cliente_udp_enviar_inicio(ClienteUDP *cliente);
int cliente_udp_enviar_dados(ClienteUDP *cliente, const char *dados, int tamanho_dados);
int cliente_udp_enviar_fim(ClienteUDP *cliente);
int cliente_udp_enviar_arquivo(ClienteUDP *cliente, const char *nome_arquivo);
void cliente_udp_fechar(ClienteUDP *cliente);
void cliente_udp_definir_verboso(ClienteUDP *cliente, int verboso);
void criar_pacote(Pacote *pacote, MessageType tipo, uint32_t num_seq, const char *dados, int tamanho_dados);

#endif