TCP: tcpCliente.c tcpServer.c hashMd5.o
		gcc tcpCliente.c hashMd5.o -o tcpCliente -lcrypto
		gcc tcpServer.c hashMd5.o -o tcpServer -lcrypto


hashMd5.o: hashMd5.h hashMd5.c
		gcc -c hashMd5.c

clean:
		rm -f *.o
		rm -f tcpCliente
		rm -f tcpServer
		rm hashMd5.txt
		rm hashRecebido.txt
		rm testeRecebido.txt