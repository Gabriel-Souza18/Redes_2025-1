# Transferencia de arquivo TCP vs UDP

## Descrição 
Este projeto implementa uma aplicação cliente-servidor em C que realiza transferência de arquivos utilizando os protocolos TCP e UDP, com o objetivo de comparar seu desempenho em termos de:

* Taxa de transferência
* Confiabilidade
* Simplicidade de implementação

## Requisitos 
* Compilador GCC
* Sistema operacional Linux

## Bibliotecas

* **sys/socket.h, netinet/in.h, arpa/inet.h**: <br>
Essenciais para a programação de sockets e comunicação em rede (TCP/IP e UDP/IP).
* **openssl/md5.h e/ou openssl/evp.h**: <br>
Fundamentais para o cálculo e verificação do hash MD5 dos arquivos
* **unistd.h**: <br>Fornece acesso a chamadas de sistema POSIX, como read, write, close, usleep, que são usadas extensivamente na manipulação de sockets e arquivos.
* **time.h**: <br>Usada para medir o tempo de execução (clock_gettime).
* **sys/stat.h, sys/types.h**:<br> Utilizadas para operações no sistema de arquivos, como a criação de diretórios (mkdir) e verificação de status de arquivos (stat).
* **libgen.h**: <br>Usada para a função basename, útil para extrair nomes de arquivos de caminhos.

## HashMD5
Explicar as funcoes dentro do hashMd5.c


## Compilação e limpeza
    make TCP
ou
  
    make UDP

para limpar arquivos gerados pela execução
    
    make clean
## Execução

Abra dois terminais, um vai ser o server e o outro o cliente 
### Server:
    ./tcpServer
ou
    
    ./udpServer

### Cliente:
    ./tcpCliente
ou

    ./udpCliente

## Autores 
Gabriel da Silva Souza \
Braian Melo Silva \
Gustavo Henrique campos