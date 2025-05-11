CC = gcc
CFLAGS = -Wall
CRYPTO = -lcrypto
SSL = -lssl

all: TCP UDP

TCP: tcpCliente.c tcpServer.c hashMd5.o
	$(CC) $(CFLAGS) tcpCliente.c hashMd5.o -o tcpCliente $(CRYPTO)
	$(CC) $(CFLAGS) tcpServer.c hashMd5.o -o tcpServer $(CRYPTO)

UDP: udpCliente.c udpServer.c hashMd5.o
	$(CC) $(CFLAGS) udpCliente.c hashMd5.o -o udpCliente $(SSL) $(CRYPTO)
	$(CC) $(CFLAGS) udpServer.c hashMd5.o -o udpServer $(CRYPTO)

hashMd5.o: hashMd5.h hashMd5.c
	$(CC) $(CFLAGS) -c hashMd5.c

TestesTCP: tcpCliente tcpServer
	./tcpCliente 127.0.0.1 8080 Arquivos/100b.txt
	./tcpCliente 127.0.0.1 8080 Arquivos/500b.txt
	./tcpCliente 127.0.0.1 8080 Arquivos/1000b.txt
	./tcpCliente 127.0.0.1 8080 Arquivos/4000b.txt
	./tcpCliente 127.0.0.1 8080 Arquivos/500000b.txt
	./tcpCliente 127.0.0.1 8080 Arquivos/5000000b.txt

TestesUDP: udpCliente udpServer
	./udpCliente 127.0.0.1 8080 Arquivos/100b.txt
	./udpCliente 127.0.0.1 8080 Arquivos/500b.txt
	./udpCliente 127.0.0.1 8080 Arquivos/1000b.txt
	./udpCliente 127.0.0.1 8080 Arquivos/4000b.txt
	./udpCliente 127.0.0.1 8080 Arquivos/500000b.txt
	./udpCliente 127.0.0.1 8080 Arquivos/5000000b.txt

clean:
	rm -f *.o
	rm -f tcpCliente tcpServer udpCliente udpServer
	rm -rf Recebidos
