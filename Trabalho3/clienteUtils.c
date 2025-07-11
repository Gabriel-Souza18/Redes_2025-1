#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/time.h>
#include "clienteUtils.h"

void cliente_udp_inicializar(ClienteUDP *cliente, const char *ip_servidor, int porta_servidor) {
    char mensagem[256];
    
    cliente->socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (cliente->socket_fd < 0) {
        sprintf(mensagem, "CLIENTE: Erro ao criar socket");
        logger(mensagem, "cliente.log");
        perror("Erro ao criar socket");
        exit(EXIT_FAILURE);
    }

    // Configurar timeout
    struct timeval timeout;
    timeout.tv_sec = TIMEOUT_SEC;
    timeout.tv_usec = 0;
    setsockopt(cliente->socket_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    memset(&cliente->endereco_servidor, 0, sizeof(cliente->endereco_servidor));
    cliente->endereco_servidor.sin_family = AF_INET;
    cliente->endereco_servidor.sin_port = htons(porta_servidor);
    cliente->num_sequencia = 0;
    cliente->timeout_seg = TIMEOUT_SEC;
    cliente->modo_verboso = 0;
    
    // Inicializar estatísticas
    memset(&cliente->estatisticas, 0, sizeof(EstatisticasCliente));
    time(&cliente->estatisticas.tempo_inicio);

    if (inet_pton(AF_INET, ip_servidor, &cliente->endereco_servidor.sin_addr) <= 0) {
        sprintf(mensagem, "CLIENTE: Endereço inválido ou não suportado");
        logger(mensagem, "cliente.log");
        perror("Endereço inválido ou não suportado");
        exit(EXIT_FAILURE);
    }
    
    sprintf(mensagem, "CLIENTE: Cliente inicializado para %s:%d", ip_servidor, porta_servidor);
    logger(mensagem, "cliente.log");
    log_com_timestamp(mensagem, cliente->modo_verboso);
}

void criar_pacote(Pacote *pacote, MessageType tipo, uint32_t num_seq, const char *dados, int tamanho_dados) {
    pacote->tipo = tipo;
    pacote->num_sequencia = num_seq;
    pacote->tamanho_dados = tamanho_dados;
    
    if (dados && tamanho_dados > 0) {
        memcpy(pacote->dados, dados, tamanho_dados);
        pacote->checksum = calcular_checksum(dados, tamanho_dados);
    } else {
        pacote->checksum = 0;
    }
}

int cliente_udp_enviar_pacote(ClienteUDP *cliente, const Pacote *pacote) {
    char mensagem[256];
    
    int bytes_sent = sendto(cliente->socket_fd, pacote, 
                           sizeof(MessageType) + sizeof(uint32_t) * 3 + pacote->tamanho_dados, 
                           0, (struct sockaddr*)&cliente->endereco_servidor, 
                           sizeof(cliente->endereco_servidor));
    if (bytes_sent < 0) {
        sprintf(mensagem, "CLIENTE: Erro ao enviar pacote");
        logger(mensagem, "cliente.log");
        perror("Erro ao enviar pacote");
        return -1;
    }
    
    cliente->estatisticas.pacotes_enviados++;
    cliente->estatisticas.total_bytes_enviados += bytes_sent;
    
    sprintf(mensagem, "CLIENTE: Pacote enviado - Tipo: %d, Seq: %u, Tamanho: %u", 
            pacote->tipo, pacote->num_sequencia, pacote->tamanho_dados);
    logger(mensagem, "cliente.log");
    log_com_timestamp(mensagem, cliente->modo_verboso);
    
    return bytes_sent;
}

int cliente_udp_receber_ack(ClienteUDP *cliente, uint32_t seq_esperada) {
    Pacote ack_pacote;
    socklen_t addr_len = sizeof(cliente->endereco_servidor);
    char mensagem[256];
    
    int bytes_received = recvfrom(cliente->socket_fd, &ack_pacote, sizeof(Pacote), 0,
                                 (struct sockaddr*)&cliente->endereco_servidor, &addr_len);
    
    if (bytes_received < 0) {
        cliente->estatisticas.timeouts++;
        sprintf(mensagem, "CLIENTE: Timeout ou erro ao receber ACK");
        logger(mensagem, "cliente.log");
        log_com_timestamp(mensagem, cliente->modo_verboso);
        return -1;
    }
    
    cliente->estatisticas.pacotes_recebidos++;
    
    if (ack_pacote.tipo == MSG_ACK && ack_pacote.num_sequencia == seq_esperada) {
        sprintf(mensagem, "CLIENTE: ACK recebido para sequência %u", seq_esperada);
        logger(mensagem, "cliente.log");
        log_com_timestamp(mensagem, cliente->modo_verboso);
        return 0;
    } else if (ack_pacote.tipo == MSG_NACK) {
        sprintf(mensagem, "CLIENTE: NACK recebido para sequência %u", seq_esperada);
        logger(mensagem, "cliente.log");
        log_com_timestamp(mensagem, cliente->modo_verboso);
        return -2;
    }
    
    sprintf(mensagem, "CLIENTE: ACK inválido recebido");
    logger(mensagem, "cliente.log");
    log_com_timestamp(mensagem, cliente->modo_verboso);
    return -1;
}

int cliente_udp_enviar_inicio(ClienteUDP *cliente) {
    Pacote pacote;
    char mensagem[256];
    criar_pacote(&pacote, MSG_START_TRANSMISSION, cliente->num_sequencia, NULL, 0);
    
    for (int tentativa = 0; tentativa < MAX_RETRIES; tentativa++) {
        sprintf(mensagem, "CLIENTE: Enviando mensagem de início de transmissão (tentativa %d)", tentativa + 1);
        logger(mensagem, "cliente.log");
        log_com_timestamp(mensagem, cliente->modo_verboso);
        
        if (cliente_udp_enviar_pacote(cliente, &pacote) < 0) {
            return -1;
        }
        
        if (cliente_udp_receber_ack(cliente, cliente->num_sequencia) == 0) {
            cliente->num_sequencia++;
            return 0;
        }
        
        if (tentativa < MAX_RETRIES - 1) {
            cliente->estatisticas.retransmissoes++;
            sprintf(mensagem, "CLIENTE: Retentativa em 1 segundo...");
            logger(mensagem, "cliente.log");
            log_com_timestamp(mensagem, cliente->modo_verboso);
            sleep(1);
        }
    }
    
    sprintf(mensagem, "CLIENTE: Falha ao enviar mensagem de início após %d tentativas", MAX_RETRIES);
    logger(mensagem, "cliente.log");
    log_com_timestamp(mensagem, cliente->modo_verboso);
    return -1;
}

int cliente_udp_enviar_dados(ClienteUDP *cliente, const char *dados, int tamanho_dados) {
    Pacote pacote;
    char mensagem[256];
    criar_pacote(&pacote, MSG_DATA, cliente->num_sequencia, dados, tamanho_dados);
    
    for (int tentativa = 0; tentativa < MAX_RETRIES; tentativa++) {
        sprintf(mensagem, "CLIENTE: Enviando dados (seq: %u, tamanho: %d, tentativa: %d)", 
               cliente->num_sequencia, tamanho_dados, tentativa + 1);
        logger(mensagem, "cliente.log");
        log_com_timestamp(mensagem, cliente->modo_verboso);
        
        if (cliente_udp_enviar_pacote(cliente, &pacote) < 0) {
            return -1;
        }
        
        if (cliente_udp_receber_ack(cliente, cliente->num_sequencia) == 0) {
            cliente->num_sequencia++;
            return 0;
        }
        
        if (tentativa < MAX_RETRIES - 1) {
            cliente->estatisticas.retransmissoes++;
            sprintf(mensagem, "CLIENTE: Retentativa em 1 segundo...");
            logger(mensagem, "cliente.log");
            log_com_timestamp(mensagem, cliente->modo_verboso);
            sleep(1);
        }
    }
    
    sprintf(mensagem, "CLIENTE: Falha ao enviar dados após %d tentativas", MAX_RETRIES);
    logger(mensagem, "cliente.log");
    log_com_timestamp(mensagem, cliente->modo_verboso);
    return -1;
}

int cliente_udp_enviar_fim(ClienteUDP *cliente) {
    Pacote pacote;
    char mensagem[256];
    criar_pacote(&pacote, MSG_END_TRANSMISSION, cliente->num_sequencia, NULL, 0);
    
    for (int tentativa = 0; tentativa < MAX_RETRIES; tentativa++) {
        sprintf(mensagem, "CLIENTE: Enviando mensagem de fim de transmissão (tentativa %d)", tentativa + 1);
        logger(mensagem, "cliente.log");
        log_com_timestamp(mensagem, cliente->modo_verboso);
        
        if (cliente_udp_enviar_pacote(cliente, &pacote) < 0) {
            return -1;
        }
        
        if (cliente_udp_receber_ack(cliente, cliente->num_sequencia) == 0) {
            cliente->num_sequencia++;
            return 0;
        }
        
        if (tentativa < MAX_RETRIES - 1) {
            cliente->estatisticas.retransmissoes++;
            sprintf(mensagem, "CLIENTE: Retentativa em 1 segundo...");
            logger(mensagem, "cliente.log");
            log_com_timestamp(mensagem, cliente->modo_verboso);
            sleep(1);
        }
    }
    
    sprintf(mensagem, "CLIENTE: Falha ao enviar mensagem de fim após %d tentativas", MAX_RETRIES);
    logger(mensagem, "cliente.log");
    log_com_timestamp(mensagem, cliente->modo_verboso);
    return -1;
}

int cliente_udp_enviar_arquivo(ClienteUDP *cliente, const char *nome_arquivo) {
    char mensagem[256];
    
    FILE *arquivo = fopen(nome_arquivo, "rb");
    if (!arquivo) {
        sprintf(mensagem, "CLIENTE: Erro ao abrir arquivo: %s", nome_arquivo);
        logger(mensagem, "cliente.log");
        perror("Erro ao abrir arquivo");
        return -1;
    }
    
    sprintf(mensagem, "CLIENTE: Iniciando envio de arquivo: %s", nome_arquivo);
    logger(mensagem, "cliente.log");
    log_com_timestamp(mensagem, cliente->modo_verboso);
    
    // Enviar mensagem de início
    if (cliente_udp_enviar_inicio(cliente) < 0) {
        fclose(arquivo);
        return -1;
    }
    
    // Enviar dados do arquivo em chunks
    char buffer[BUFFER_SIZE - sizeof(MessageType) - sizeof(uint32_t) * 3];
    size_t bytes_lidos;
    int chunk_count = 0;
    
    while ((bytes_lidos = fread(buffer, 1, sizeof(buffer), arquivo)) > 0) {
        chunk_count++;
        sprintf(mensagem, "CLIENTE: Enviando chunk %d (%zu bytes)", chunk_count, bytes_lidos);
        logger(mensagem, "cliente.log");
        log_com_timestamp(mensagem, cliente->modo_verboso);
        
        if (cliente_udp_enviar_dados(cliente, buffer, bytes_lidos) < 0) {
            fclose(arquivo);
            return -1;
        }
    }
    
    // Enviar mensagem de fim
    if (cliente_udp_enviar_fim(cliente) < 0) {
        fclose(arquivo);
        return -1;
    }
    
    fclose(arquivo);
    time(&cliente->estatisticas.tempo_fim);
    
    sprintf(mensagem, "CLIENTE: Arquivo enviado com sucesso - %d chunks enviados", chunk_count);
    logger(mensagem, "cliente.log");
    log_com_timestamp(mensagem, cliente->modo_verboso);
    return 0;
}

void cliente_udp_definir_verboso(ClienteUDP *cliente, int verboso) {
    char mensagem[64];
    cliente->modo_verboso = verboso;
    sprintf(mensagem, "CLIENTE: Modo verbose %s", verboso ? "ativado" : "desativado");
    logger(mensagem, "cliente.log");
    log_com_timestamp(mensagem, verboso);
}

void cliente_udp_fechar(ClienteUDP *cliente) {
    char mensagem[64];
    close(cliente->socket_fd);
    time(&cliente->estatisticas.tempo_fim);
    sprintf(mensagem, "CLIENTE: Cliente UDP fechado");
    logger(mensagem, "cliente.log");
    log_com_timestamp(mensagem, cliente->modo_verboso);
}