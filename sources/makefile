all: client serveur

client: client.c
	gcc client.c common.o -o client -Wall -lpthread

serveur: serveur.c 
	gcc serveur.c common.o -o serveur -Wall -lpthread

clean:
	rm  serveur client
	clear
				
