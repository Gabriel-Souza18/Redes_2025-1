# Transferencia de arquivo confiavel UDP

## Descrição 
Implementação de um protocolo de parada e espera confiavel usando UDP, simulando um ambiente com perda
de pacotes. O protocolo deve garantir entrega correta e completa de um arquivo entre cliente e servidor, uti-
lizando ACKs, retransmissão, controle de sequência e timeout. O trabalho tambem deve incluir ferramentas
de diagnostico e estatisticas de desempenho

## Requisitos 
* Compilador GCC
* Sistema operacional Linux

## Bibliotecas

* **sys/socket.h, netinet/in.h, arpa/inet.h**: <br>
Essenciais para a programação de sockets e comunicação em rede 
* **time.h**: <br>Usada para medir o tempo de execução (clock_gettime).
* **sys/stat.h, sys/types.h**:<br> Utilizadas para operações no sistema de arquivos, como a criação de diretórios (mkdir) e verificação de status de arquivos (stat).


## Compilação e limpeza
    make server
    make cliente

para limpar arquivos gerados pela execução
    
    make clean

### Server:
    
    ./server (Porta)

### Cliente:
    ./cliente (IP) (Porta) (Caminho do arquivo)

## Autores 
Gabriel da Silva Souza \
Braian Melo Silva \
Gustavo Henrique campos