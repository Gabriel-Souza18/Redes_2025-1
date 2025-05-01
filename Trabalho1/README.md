# Transferencia de arquivo TCP vs UDP

## Descrição 
Este projeto implementa uma aplicação cliente-servidor em C que realiza transferência de arquivos utilizando os protocolos TCP e UDP, com o objetivo de comparar seu desempenho em termos de:

* Taxa de transferência
* Confiabilidade
* Simplicidade de implementação

## Requisitos 
* Compilador GCC
* Sistema operacional Linux


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