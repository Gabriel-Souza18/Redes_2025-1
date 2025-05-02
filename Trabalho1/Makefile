all: TCP UDP

TCP: tcpCliente.c tcpServer.c hashMd5.o
		gcc tcpCliente.c hashMd5.o -o tcpCliente -lcrypto
		gcc tcpServer.c hashMd5.o -o tcpServer -lcrypto
		
		
testesTCP: tcpCliente tcpServer
		./tcpCliente Arquivos/100b.txt
		./tcpCliente Arquivos/200b.txt
		./tcpCliente Arquivos/500b.txt
		./tcpCliente Arquivos/1000b.txt
		./tcpCliente Arquivos/2000b.txt
		./tcpCliente Arquivos/3000b.txt
		./tcpCliente Arquivos/4000b.txt

UDP: udpCliente.c udpServer.c hashMd5.o
		gcc udpCliente.c hashMd5.o -o udpCliente -lssl -lcrypto
		gcc udpServer.c hashMd5.o -o udpServer -lcrypto
		
testesUDP: udpCliente udpServer
		./udpCliente Arquivos/100b.txt
		./udpCliente Arquivos/200b.txt
		./udpCliente Arquivos/500b.txt
		./udpCliente Arquivos/1000b.txt
		./udpCliente Arquivos/2000b.txt
		./udpCliente Arquivos/3000b.txt
		./udpCliente Arquivos/4000b.txt

hashMd5.o: hashMd5.h hashMd5.c
		gcc -c hashMd5.c

clean:
		rm -f *.o
		rm -f tcpCliente
		rm -f tcpServer
		rm -r Recebidos
		rm -f udpCliente
		rm -f udpServer
