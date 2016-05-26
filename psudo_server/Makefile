psudo_server: server.o 
	gcc  -o psudo_server server.o 
server.o: server.c server.h
	gcc  -c -g server.c
clean:
	rm --force psudo_server
	rm --force *.o
	rm --force *~
	rm --force *bak

