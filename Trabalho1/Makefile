all: TCP UDP

TCP: tcpCliente.c tcpServer.c hashMd5.o
		gcc tcpCliente.c hashMd5.o -o tcpCliente -lcrypto
		gcc tcpServer.c hashMd5.o -o tcpServer -lcrypto
		
		
testesTCP: tcpCliente tcpServer
		./tcpCliente 127.0.0.1 8080 Arquivos/100b.txt # 100 bytes
		./tcpCliente 127.0.0.1 8080 Arquivos/500b.txt # 500 bytes
		./tcpCliente 127.0.0.1 8080 Arquivos/1000b.txt # 1k bytes
		./tcpCliente 127.0.0.1 8080 Arquivos/4000b.txt # 4k bytes
		./tcpCliente 127.0.0.1 8080 Arquivos/500000b.txt 	# 500k bytes
		./tcpCliente 127.0.0.1 8080 Arquivos/5000000b.txt  # 5M bytes

UDP: udpCliente.c udpServer.c hashMd5.o
		gcc udpCliente.c hashMd5.o -o udpCliente -lssl -lcrypto
		gcc udpServer.c hashMd5.o -o udpServer -lcrypto
		
testesUDP: udpCliente udpServer
		./udpCliente 127.0.0.1 8080 Arquivos/100b.txt # 100 bytes
		./udpCliente 127.0.0.1 8080 Arquivos/500b.txt # 500 bytes
		./udpCliente 127.0.0.1 8080 Arquivos/1000b.txt # 1k bytes
		./udpCliente 127.0.0.1 8080 Arquivos/4000b.txt # 4k bytes
		./udpCliente 127.0.0.1 8080 Arquivos/500000b.txt 	# 500k bytes
		./udpCliente 127.0.0.1 8080 Arquivos/5000000b.txt  # 5M bytes


hashMd5.o: hashMd5.h hashMd5.c
		gcc -c hashMd5.c

clean:
		rm -f *.o
		rm -f tcpCliente
		rm -f tcpServer
		rm -f udpCliente
		rm -f udpServer
		rm -rf Recebidos
		rm hashRecebido.txt
