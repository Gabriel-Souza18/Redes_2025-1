#include "serverUtils.h"
#include "utils.h"
#include <stdlib.h>
#include <time.h>

void imprimir_uso(const char *nome_programa) {
    printf("Uso: %s [OPÇÕES] [porta]\n", nome_programa);
    printf("Opções:\n");
    printf("  -v, --verbose       Ativar modo verbose\n");
    printf("  -l, --loss TAXA     Taxa de perda de pacotes (0.0-1.0)\n");
    printf("  -h, --help          Mostrar esta ajuda\n");
    printf("\nExemplo:\n");
    printf("  %s -v -l 0.1 8080   # Modo verbose, 10%% de perda, porta 8080\n", nome_programa);
}

int main(int argc, char *argv[]) {
    ServidorUDP servidor;
    int porta = 8080;
    int modo_verboso = 0;
    double taxa_perda = 0.0;
    
    // Inicializar gerador de números aleatórios
    srand(time(NULL));
    
    // Processar argumentos da linha de comando
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) {
            modo_verboso = 1;
        } else if (strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "--loss") == 0) {
            if (i + 1 < argc) {
                taxa_perda = atof(argv[++i]);
                if (taxa_perda < 0.0 || taxa_perda > 1.0) {
                    fprintf(stderr, "Erro: taxa de perda deve estar entre 0.0 e 1.0\n");
                    return EXIT_FAILURE;
                }
            } else {
                fprintf(stderr, "Erro: taxa de perda não especificada\n");
                return EXIT_FAILURE;
            }
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            imprimir_uso(argv[0]);
            return EXIT_SUCCESS;
        } else {
            // Assumir que é a porta
            porta = atoi(argv[i]);
            if (porta <= 0 || porta > 65535) {
                fprintf(stderr, "Erro: porta inválida (%d)\n", porta);
                return EXIT_FAILURE;
            }
        }
    }
    
    servidor_udp_inicializar(&servidor);
    servidor_udp_definir_verboso(&servidor, modo_verboso);
    servidor_udp_definir_taxa_perda(&servidor, taxa_perda);
    
    if (servidor_udp_abrir(&servidor, porta) < 0) {
        return EXIT_FAILURE;
    }

    printf("Servidor UDP rodando na porta %d...\n", porta);
    printf("Modo verbose: %s\n", modo_verboso ? "Ativo" : "Inativo");
    printf("Taxa de perda simulada: %.1f%%\n", taxa_perda * 100);
    
    logger("Servidor UDP iniciado", "server.log");
    servidor_udp_imprimir_info(&servidor);
    
    while (1) {
        printf("Aguardando nova transmissão...\n");
        
        if (servidor_udp_processar_transmissao(&servidor) == 0) {
            printf("Transmissão processada com sucesso!\n");
            logger("Transmissão completa recebida", "server.log");
            
            // Mostrar estatísticas usando função do utils
            imprimir_estatisticas_servidor(
                servidor.estatisticas.pacotes_recebidos,
                servidor.estatisticas.pacotes_enviados,
                servidor.estatisticas.acks_enviados,
                servidor.estatisticas.nacks_enviados,
                servidor.estatisticas.pacotes_corrompidos,
                servidor.estatisticas.pacotes_duplicados,
                servidor.estatisticas.tempo_inicio,
                time(NULL)  // Tempo atual como fim
            );
            
            // Reset da sequência para próxima transmissão
            servidor.sequencia_esperada = 0;
        }
    }
    
    servidor_udp_fechar(&servidor);
    logger("Servidor UDP fechado", "server.log");
    return 0;
}