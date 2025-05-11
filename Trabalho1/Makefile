all: TCP UDP

TCP: tcpCliente.c tcpServer.c hashMd5.o
		gcc tcpCliente.c hashMd5.o -o tcpCliente -lcrypto
		gcc tcpServer.c hashMd5.o -o tcpServer -lcrypto
		
		
testesTCP: tcpCliente tcpServer
		./tcpCliente Arquivos/100b.txt # 100 bytes
		./tcpCliente Arquivos/500b.txt # 500 bytes
		./tcpCliente Arquivos/1000b.txt # 1k bytes
		./tcpCliente Arquivos/4000b.txt # 4k bytes
		./tcpCliente Arquivos/500000b.txt 	# 500k bytes
		./tcpCliente Arquivos/1000000b.txt  # 1M bytes
		./tcpCliente Arquivos/5000000b.txt  # 5M bytes
		./tcpCliente Arquivos/1000000000b.txt # 1G bytes

UDP: udpCliente.c udpServer.c hashMd5.o
		gcc udpCliente.c hashMd5.o -o udpCliente -lssl -lcrypto
		gcc udpServer.c hashMd5.o -o udpServer -lcrypto
		
testesUDP: udpCliente udpServer
		./udpCliente Arquivos/100b.txt # 100 bytes
		./udpCliente Arquivos/500b.txt # 500 bytes
		./udpCliente Arquivos/1000b.txt # 1k bytes
		./udpCliente Arquivos/4000b.txt # 4k bytes
		./udpCliente Arquivos/500000b.txt 	# 500k bytes
		./udpCliente Arquivos/1000000b.txt  # 1M bytes
		./udpCliente Arquivos/5000000b.txt  # 5M bytes
		./udpCliente Arquivos/1000000000b.txt # 1G bytes


hashMd5.o: hashMd5.h hashMd5.c
		gcc -c hashMd5.c

clean:
		rm -f *.o
		rm -f tcpCliente
		rm -f tcpServer
		rm -r Recebidos
		rm -f udpCliente
		rm -f udpServer
