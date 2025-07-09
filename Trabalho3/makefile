

Server: server.c utils.o serverUtils.o
	gcc -o server server.c utils.o serverUtils.o 

utils.o: utils.c utils.h
	gcc -c utils.c

serverUtils.o: serverUtils.c serverUtils.h
	gcc -c serverUtils.c

clean:
	rm -f server 
	rm -f *.o
	rm -f  server.log
