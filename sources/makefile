all: client serveur

client: client.c
	gcc client.c common.o -o client -Wall

serveur: serveur.c 
	gcc serveur.c common.o -o serveur -Wall

clean:	
	./destruction
	rm  serveur client
	clear
				