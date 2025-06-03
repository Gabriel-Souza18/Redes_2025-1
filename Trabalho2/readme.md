# Servidor Web

## Decrição
Este projeto implementa um servidor web em C com qautro técnicas distintas de programação concorrente:
* Servidor iterativo
* Servidor com fork/threads
* Servidor com thread pool e fila de tarefas
* Servidor concorrente usando select()

## Requisitos
* Compilador GCC
* Linux

## Bibliotecas 
```
#include <sys/socket.h>   // Sockets e operações de rede
#include <netinet/in.h>    // Estruturas de endereçamento IP
#include <pthread.h>       // Threads POSIX
#include <unistd.h>        // Chamadas de sistema (read, write, close)
#include <sys/select.h>    // Multiplexação I/O (select)
#include <sys/stat.h>      // Operações com arquivos
#include <fcntl.h>         // Controle de arquivos
#include <time.h>          // Manipulação de tempo
```

## Estrutura do projeto
```
Trabalho2
├── src/
│   ├── iterativo.c
│   ├── fork_thread.c
│   ├── threadpool.c
│   └── select_server.c
├── www/
│   ├── index.html
│   ├── image.jpg
│   └── document.pdf
├── documentação
└── Makefile
```
## Compilação 
Compilar todos:
```
make all
```
Compilar servidor especifico:
```
make iterative    # Abordagem 1
make fork_thread  # Abordagem 2
make threadpool   # Abordagem 3
make select       # Abordagem 4
```
Limpar:
```
make clean
```

## Autores 
Gabriel da Silva Souza \
Braian Melo Silva \
Gustavo Henrique campos

