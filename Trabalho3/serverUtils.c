#include "serverUtils.h"

void servidor_udp_inicializar(ServidorUDP *servidor) {
    servidor->socket_fd = -1;
    servidor->porta = DEFAULT_PORT;
    servidor->esta_aberto = 0;
    servidor->tamanho_cliente = sizeof(servidor->endereco_cliente);
    servidor->sequencia_esperada = 0;
    servidor->modo_verboso = 0;
    servidor->taxa_perda = 0.0;
    
    // Inicializar estatísticas
    memset(&servidor->estatisticas, 0, sizeof(EstatisticasServidor));
    
    memset(&servidor->endereco_servidor, 0, sizeof(servidor->endereco_servidor));
    memset(&servidor->endereco_cliente, 0, sizeof(servidor->endereco_cliente));
}

void servidor_udp_definir_verboso(ServidorUDP *servidor, int verboso) {
    servidor->modo_verboso = verboso;
}

void servidor_udp_definir_taxa_perda(ServidorUDP *servidor, double taxa_perda) {
    servidor->taxa_perda = taxa_perda;
}

int servidor_udp_abrir(ServidorUDP *servidor, int porta) {
    if (servidor->esta_aberto) {
        log_com_timestamp("SERVIDOR: Servidor já está aberto", servidor->modo_verboso);
        return -1;
    }

    // Criar socket UDP
    servidor->socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (servidor->socket_fd < 0) {
        perror("Erro ao criar socket");
        return -1;
    }

    // Configurar endereço do servidor
    servidor->endereco_servidor.sin_family = AF_INET;
    servidor->endereco_servidor.sin_addr.s_addr = INADDR_ANY;
    servidor->endereco_servidor.sin_port = htons(porta);
    servidor->porta = porta;

    // Fazer bind do socket
    if (bind(servidor->socket_fd, (struct sockaddr*)&servidor->endereco_servidor, 
             sizeof(servidor->endereco_servidor)) < 0) {
        perror("Erro no bind");
        close(servidor->socket_fd);
        return -1;
    }

    servidor->esta_aberto = 1;
    time(&servidor->estatisticas.tempo_inicio);
    
    char mensagem[256];
    sprintf(mensagem, "SERVIDOR: Servidor UDP aberto na porta %d", porta);
    log_com_timestamp(mensagem, servidor->modo_verboso);
    logger(mensagem, "server.log");
    
    return 0;
}

int servidor_udp_fechar(ServidorUDP *servidor) {
    if (!servidor->esta_aberto) {
        log_com_timestamp("SERVIDOR: Servidor não está aberto", servidor->modo_verboso);
        return -1;
    }

    if (close(servidor->socket_fd) < 0) {
        perror("Erro ao fechar socket");
        return -1;
    }

    servidor->esta_aberto = 0;
    servidor->socket_fd = -1;
    time(&servidor->estatisticas.tempo_fim);
    
    log_com_timestamp("SERVIDOR: Servidor UDP fechado", servidor->modo_verboso);
    logger("Servidor UDP fechado", "server.log");
    return 0;
}

int servidor_udp_receber_pacote(ServidorUDP *servidor, Pacote *pacote) {
    if (!servidor->esta_aberto) {
        log_com_timestamp("SERVIDOR: Servidor não está aberto", servidor->modo_verboso);
        return -1;
    }

    servidor->tamanho_cliente = sizeof(servidor->endereco_cliente);
    int bytes_received = recvfrom(servidor->socket_fd, pacote, sizeof(Pacote), 0,
                                  (struct sockaddr*)&servidor->endereco_cliente, 
                                  &servidor->tamanho_cliente);
    
    if (bytes_received < 0) {
        perror("Erro ao receber dados");
        return -1;
    }

    servidor->estatisticas.pacotes_recebidos++;
    
    char mensagem[256];
    sprintf(mensagem, "SERVIDOR: Pacote recebido - Tipo: %d, Seq: %u, Tamanho: %u", 
            pacote->tipo, pacote->num_sequencia, pacote->tamanho_dados);
    log_com_timestamp(mensagem, servidor->modo_verboso);

    return bytes_received;
}

int servidor_udp_enviar_ack(ServidorUDP *servidor, uint32_t num_sequencia) {
    // Simular perda de ACK usando função do utils
    if (simular_perda_pacote(servidor->taxa_perda)) {
        char mensagem[256];
        sprintf(mensagem, "SERVIDOR: ACK perdido simuladamente para sequência %u", num_sequencia);
        log_com_timestamp(mensagem, servidor->modo_verboso);
        logger(mensagem, "server.log");
        return 0; // Simula que foi enviado mas perdido
    }
    
    Pacote ack_packet;
    ack_packet.tipo = MSG_ACK;
    ack_packet.num_sequencia = num_sequencia;
    ack_packet.tamanho_dados = 0;
    ack_packet.checksum = 0;

    int bytes_sent = sendto(servidor->socket_fd, &ack_packet, 
                           sizeof(MessageType) + sizeof(uint32_t) * 3, 0,
                           (struct sockaddr*)&servidor->endereco_cliente, 
                           sizeof(servidor->endereco_cliente));
    
    if (bytes_sent < 0) {
        perror("Erro ao enviar ACK");
        return -1;
    }

    servidor->estatisticas.pacotes_enviados++;
    servidor->estatisticas.acks_enviados++;
    
    char mensagem[256];
    sprintf(mensagem, "SERVIDOR: ACK enviado para sequência %u", num_sequencia);
    log_com_timestamp(mensagem, servidor->modo_verboso);
    
    return bytes_sent;
}

int servidor_udp_enviar_nack(ServidorUDP *servidor, uint32_t num_sequencia) {
    Pacote nack_packet;
    nack_packet.tipo = MSG_NACK;
    nack_packet.num_sequencia = num_sequencia;
    nack_packet.tamanho_dados = 0;
    nack_packet.checksum = 0;

    int bytes_sent = sendto(servidor->socket_fd, &nack_packet, 
                           sizeof(MessageType) + sizeof(uint32_t) * 3, 0,
                           (struct sockaddr*)&servidor->endereco_cliente, 
                           sizeof(servidor->endereco_cliente));
    
    if (bytes_sent < 0) {
        perror("Erro ao enviar NACK");
        return -1;
    }

    servidor->estatisticas.pacotes_enviados++;
    servidor->estatisticas.nacks_enviados++;
    
    char mensagem[256];
    sprintf(mensagem, "SERVIDOR: NACK enviado para sequência %u", num_sequencia);
    log_com_timestamp(mensagem, servidor->modo_verboso);
    
    return bytes_sent;
}

int servidor_udp_processar_transmissao(ServidorUDP *servidor) {
    Pacote pacote;
    int transmissao_ativa = 0;
    uint32_t ultima_seq_recebida = UINT32_MAX;
    
    while (1) {
        int bytes_received = servidor_udp_receber_pacote(servidor, &pacote);
        if (bytes_received < 0) {
            continue;
        }
        
        // Verificar integridade do pacote usando função do utils
        if (pacote.tamanho_dados > 0 && !verificar_integridade_pacote(pacote.dados, pacote.checksum, pacote.tamanho_dados)) {
            servidor->estatisticas.pacotes_corrompidos++;
            log_com_timestamp("SERVIDOR: Pacote corrompido detectado - descartando", servidor->modo_verboso);
            logger("Pacote corrompido descartado", "server.log");
            continue;
        }
        
        // Verificar pacote duplicado
        if (pacote.num_sequencia == ultima_seq_recebida) {
            servidor->estatisticas.pacotes_duplicados++;
            char mensagem[256];
            sprintf(mensagem, "SERVIDOR: Pacote duplicado detectado - Seq: %u", pacote.num_sequencia);
            log_com_timestamp(mensagem, servidor->modo_verboso);
            logger(mensagem, "server.log");
            
            // Reenviar ACK para pacote duplicado
            if (pacote.tipo != MSG_ACK && pacote.tipo != MSG_NACK) {
                servidor_udp_enviar_ack(servidor, pacote.num_sequencia);
            }
            continue;
        }
        
        switch (pacote.tipo) {
            case MSG_START_TRANSMISSION:
                if (pacote.num_sequencia == servidor->sequencia_esperada) {
                    log_com_timestamp("SERVIDOR: === INÍCIO DE TRANSMISSÃO ===", servidor->modo_verboso);
                    logger("Início de transmissão recebido", "server.log");
                    transmissao_ativa = 1;
                    servidor_udp_enviar_ack(servidor, pacote.num_sequencia);
                    ultima_seq_recebida = pacote.num_sequencia;
                    servidor->sequencia_esperada++;
                } else {
                    char mensagem[256];
                    sprintf(mensagem, "SERVIDOR: Sequência incorreta no início. Esperado: %u, Recebido: %u", 
                           servidor->sequencia_esperada, pacote.num_sequencia);
                    log_com_timestamp(mensagem, servidor->modo_verboso);
                    logger(mensagem, "server.log");
                    servidor_udp_enviar_nack(servidor, pacote.num_sequencia);
                }
                break;
                
            case MSG_DATA:
                if (!transmissao_ativa) {
                    log_com_timestamp("SERVIDOR: Dados recebidos sem início de transmissão", servidor->modo_verboso);
                    logger("Dados recebidos sem início de transmissão", "server.log");
                    servidor_udp_enviar_nack(servidor, pacote.num_sequencia);
                } else if (pacote.num_sequencia == servidor->sequencia_esperada) {
                    char mensagem[256];
                    sprintf(mensagem, "SERVIDOR: Dados recebidos (%u bytes) - Seq: %u", 
                           pacote.tamanho_dados, pacote.num_sequencia);
                    log_com_timestamp(mensagem, servidor->modo_verboso);
                    
                    // Log dos dados apenas se modo verbose estiver ativo
                    if (servidor->modo_verboso && pacote.tamanho_dados > 0) {
                        printf("Conteúdo: %.*s\n", (int)pacote.tamanho_dados, pacote.dados);
                    }
                    
                    servidor_udp_enviar_ack(servidor, pacote.num_sequencia);
                    ultima_seq_recebida = pacote.num_sequencia;
                    servidor->sequencia_esperada++;
                } else {
                    char mensagem[256];
                    sprintf(mensagem, "SERVIDOR: Sequência incorreta nos dados. Esperado: %u, Recebido: %u", 
                           servidor->sequencia_esperada, pacote.num_sequencia);
                    log_com_timestamp(mensagem, servidor->modo_verboso);
                    logger(mensagem, "server.log");
                    servidor_udp_enviar_nack(servidor, pacote.num_sequencia);
                }
                break;
                
            case MSG_END_TRANSMISSION:
                if (pacote.num_sequencia == servidor->sequencia_esperada) {
                    log_com_timestamp("SERVIDOR: === FIM DE TRANSMISSÃO ===", servidor->modo_verboso);
                    logger("Fim de transmissão recebido", "server.log");
                    transmissao_ativa = 0;
                    servidor_udp_enviar_ack(servidor, pacote.num_sequencia);
                    ultima_seq_recebida = pacote.num_sequencia;
                    servidor->sequencia_esperada++;
                    return 0; // Transmissão completa
                } else {
                    char mensagem[256];
                    sprintf(mensagem, "SERVIDOR: Sequência incorreta no fim. Esperado: %u, Recebido: %u", 
                           servidor->sequencia_esperada, pacote.num_sequencia);
                    log_com_timestamp(mensagem, servidor->modo_verboso);
                    logger(mensagem, "server.log");
                    servidor_udp_enviar_nack(servidor, pacote.num_sequencia);
                }
                break;
                
            default:
                char mensagem[256];
                sprintf(mensagem, "SERVIDOR: Tipo de mensagem desconhecido: %d", pacote.tipo);
                log_com_timestamp(mensagem, servidor->modo_verboso);
                logger(mensagem, "server.log");
                servidor_udp_enviar_nack(servidor, pacote.num_sequencia);
                break;
        }
    }
}

void servidor_udp_imprimir_info(ServidorUDP *servidor) {
    printf("=== Informações do Servidor UDP ===\n");
    printf("Status: %s\n", servidor->esta_aberto ? "Aberto" : "Fechado");
    printf("Porta: %d\n", servidor->porta);
    printf("Socket FD: %d\n", servidor->socket_fd);
    printf("Sequência esperada: %u\n", servidor->sequencia_esperada);
    printf("Modo verbose: %s\n", servidor->modo_verboso ? "Ativo" : "Inativo");
    printf("Taxa de perda: %.1f%%\n", servidor->taxa_perda * 100);
    
    if (servidor->esta_aberto) {
        printf("Endereço: %s:%d\n", 
               inet_ntoa(servidor->endereco_servidor.sin_addr), 
               ntohs(servidor->endereco_servidor.sin_port));
    }
    printf("===================================\n");
}