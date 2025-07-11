#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include "clienteUtils.h"

void imprimir_uso(const char *nome_programa) {
    printf("Uso: %s [OPÇÕES] <servidor_ip> <porta> <arquivo>\n", nome_programa);
    printf("Opções:\n");
    printf("  -v, --verbose    Ativar modo verbose\n");
    printf("  -h, --help       Mostrar esta ajuda\n");
    printf("\nExemplo:\n");
    printf("  %s -v 127.0.0.1 8080 arquivo.txt\n", nome_programa);
}

int main(int argc, char *argv[]) {
    ClienteUDP cliente;
    char *ip_servidor = NULL;
    int porta_servidor = 0;
    char *nome_arquivo = NULL;
    int modo_verboso = 0;
    
    // Processar argumentos da linha de comando
    int indice_arg = 1;
    while (indice_arg < argc) {
        if (strcmp(argv[indice_arg], "-v") == 0 || strcmp(argv[indice_arg], "--verbose") == 0) {
            modo_verboso = 1;
            indice_arg++;
        } else if (strcmp(argv[indice_arg], "-h") == 0 || strcmp(argv[indice_arg], "--help") == 0) {
            imprimir_uso(argv[0]);
            return EXIT_SUCCESS;
        } else {
            break;
        }
    }
    
    // Verificar argumentos obrigatórios
    if (argc - indice_arg < 3) {
        fprintf(stderr, "Erro: argumentos insuficientes.\n");
        imprimir_uso(argv[0]);
        return EXIT_FAILURE;
    }
    
    ip_servidor = argv[indice_arg];
    porta_servidor = atoi(argv[indice_arg + 1]);
    nome_arquivo = argv[indice_arg + 2];
    
    if (porta_servidor <= 0 || porta_servidor > 65535) {
        fprintf(stderr, "Erro: porta inválida (%d)\n", porta_servidor);
        return EXIT_FAILURE;
    }
    
    // Verificar se arquivo existe
    FILE *teste_arquivo = fopen(nome_arquivo, "rb");
    if (!teste_arquivo) {
        fprintf(stderr, "Erro: não foi possível abrir o arquivo '%s'\n", nome_arquivo);
        return EXIT_FAILURE;
    }
    fclose(teste_arquivo);
    
    // Inicializar cliente
    cliente_udp_inicializar(&cliente, ip_servidor, porta_servidor);
    cliente_udp_definir_verboso(&cliente, modo_verboso);
    
    printf("Cliente UDP iniciado\n");
    printf("Servidor: %s:%d\n", ip_servidor, porta_servidor);
    printf("Arquivo: %s\n", nome_arquivo);
    printf("Modo verbose: %s\n", modo_verboso ? "Ativo" : "Inativo");
    
    // Enviar arquivo
    if (cliente_udp_enviar_arquivo(&cliente, nome_arquivo) < 0) {
        fprintf(stderr, "Falha ao enviar arquivo\n");
        cliente_udp_fechar(&cliente);
        return EXIT_FAILURE;
    }
    
    printf("Arquivo enviado com sucesso!\n");
    
    // Imprimir estatísticas
    imprimir_estatisticas_cliente(
        cliente.estatisticas.pacotes_enviados,
        cliente.estatisticas.pacotes_recebidos,
        cliente.estatisticas.retransmissoes,
        cliente.estatisticas.timeouts,
        cliente.estatisticas.total_bytes_enviados,
        cliente.estatisticas.tempo_inicio,
        cliente.estatisticas.tempo_fim
    );
    
    cliente_udp_fechar(&cliente);
    return EXIT_SUCCESS;
}