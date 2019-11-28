all: client serveur

client: client.c common.o common.h
	gcc client.c common.o -o client -Wall -lpthread

serveur: serveur.c common.o common.h
	gcc serveur.c common.o -o serveur -Wall -lpthread

common.o: common.c common.h
	gcc -c common.c -Wall

clean:
	rm -rf common.o serveur client
	clear
				
