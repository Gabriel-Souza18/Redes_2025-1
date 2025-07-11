server: server.c utils.o serverUtils.o
	gcc -o server server.c utils.o serverUtils.o 

utils.o: utils.c utils.h
	gcc -c utils.c

serverUtils.o: serverUtils.c serverUtils.h
	gcc -c serverUtils.c

cliente: cliente.c clienteUtils.o utils.o
	gcc -o cliente cliente.c clienteUtils.o utils.o

clienteUtils.o: clienteUtils.c clienteUtils.h
	gcc -c clienteUtils.c

clean:
	rm -f server
	rm -f cliente
	rm -f *.o
	rm -f *.log
	rm -f *.txt


