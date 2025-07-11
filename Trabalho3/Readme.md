# Transferência de Arquivo Confiável UDP

## Descrição 
Implementação de um protocolo de parada e espera confiável usando UDP, simulando um ambiente com perda de pacotes. O protocolo garante entrega correta e completa de um arquivo entre cliente e servidor, utilizando ACKs, retransmissão, controle de sequência e timeout. O trabalho também inclui ferramentas de diagnóstico e estatísticas de desempenho.

## Funcionalidades
- **Protocolo Stop-and-Wait**: Garantia de entrega ordenada de pacotes
- **Detecção de Corrupção**: Implementação de checksum para validação de integridade
- **Retransmissão Automática**: Sistema de timeout e reenvio de pacotes perdidos
- **Simulação de Perda**: Taxa configurável de perda de pacotes para testes
- **Logs Detalhados**: Registro completo de todas as operações com timestamp
- **Estatísticas de Performance**: Métricas de throughput, taxa de sucesso e latência
- **Detecção de Duplicatas**: Identificação e descarte de pacotes duplicados

## Requisitos 
- Compilador GCC
- Sistema operacional Linux
- Arquivos de teste (pode usar o gerador Python incluído)

## Bibliotecas Utilizadas

- **sys/socket.h, netinet/in.h, arpa/inet.h**: Essenciais para programação de sockets e comunicação em rede
- **time.h**: Usada para timestamps e medição de tempo de execução
- **stdio.h, stdlib.h, string.h**: Bibliotecas padrão C para I/O, memória e strings
- **unistd.h**: Para funções do sistema Unix (close, sleep)

## Compilação

```bash
# Compilar servidor
make server

# Compilar cliente  
make cliente

# Compilar ambos
make server cliente

# Limpar arquivos gerados
make clean
```

## Uso do Programa

### Servidor

```bash
./server [OPÇÕES] [porta]
```

**Opções:**
- `-v, --verbose`: Ativar modo verbose (mostra logs detalhados no console)
- `-l, --loss TAXA`: Taxa de perda de pacotes simulada (0.0-1.0)
- `-h, --help`: Mostrar ajuda

**Exemplos:**
```bash
# Servidor básico na porta 8080
./server 8080

# Servidor com modo verbose na porta 8080
./server -v 8080

# Servidor com 10% de perda de pacotes e modo verbose
./server -v -l 0.1 8080

```

### Cliente

```bash
./cliente [OPÇÕES] <servidor_ip> <porta> <arquivo>
```

**Opções:**
- `-v, --verbose`: Ativar modo verbose
- `-h, --help`: Mostrar ajuda

**Exemplos:**
```bash
# Envio básico de arquivo
./cliente 127.0.0.1 8080 arquivo.txt

# Envio com modo verbose ativo
./cliente -v 127.0.0.1 8080 arquivo.txt

```

## Geração de Arquivos de Teste (Não sei se deixo)

O projeto inclui um script Python para gerar arquivos de teste:

```bash
cd "gerador arquivo"
python3 gerador.py
```

O script gera um arquivo de 100KB com caracteres aleatórios. Você pode modificar a variável `tam` para alterar o tamanho.

## Fluxo de Execução

1. **Iniciar o servidor:**
   ```bash
   ./server -v -l 0.1 8080
   ```

2. **Em outro terminal, enviar arquivo:**
   ```bash
   ./cliente -v 127.0.0.1 8080 arquivo_100000.txt
   ```

3. **Visualizar logs:**
   ```bash
   # Logs do servidor
   cat server.log
   
   # Logs do cliente
   cat cliente.log
   ```

## Arquivos de Log

- **server.log**: Contém todos os eventos do servidor com timestamp
- **cliente.log**: Contém todos os eventos do cliente com timestamp

Os logs incluem:
- Início/fim de transmissões
- Pacotes enviados/recebidos
- ACKs e NACKs
- Timeouts e retransmissões
- Pacotes corrompidos ou duplicados
- Estatísticas finais


## Autores 
- Gabriel da Silva Souza
- Braian Melo Silva  
- Gustavo Henrique Campos