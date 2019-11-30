#ifndef COMMON_H
#define COMMON_H

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/socket.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>
#include <getopt.h>
#include <stdarg.h>
#include <unistd.h>
#include <limits.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "calcul.h"

#define N 280

char NOM_PRGRM[N];

/** 
* les structures union fonctionnent sur le principe 
* d'un XOR.
**/
union sem_union{
    int val; /* cmd = SETVAL */
    struct semid_ds * buf; /* cmd = IPC_STAT ou IPC_SET */
    unsigned short * array; /* cmd = GETALL ou SETALL */
    struct seminfo * __buf; /* cmd = IPC_INFO */
};

/**
* Pour la gestion multithreadee
**/
struct infosClient{
    int socketClient;
    int posClient;
    struct sockaddr adresseClient;
    struct infosClient * client_suivant;
};

/**
* structure permettant les envois de messages avec ce qu'on
* veut a l'interieur, en TCP et UDP
**/
struct message{
    int numero;
    char pseudo[N];
    char text[N];
};

struct memoirePartagee{
    int nbPosition;
    int position;
    struct message * commentaire;
    struct memoirePartagee * suivant;
};

/**
* renvoie une clef en fonction d'un fichier
* et d'un entier
**/
key_t getKey(int myInt);

/**
* Initialise une zone de memoire partagee
* renvoi l'identifiant de cette zone
**/
int init_shm(key_t key, size_t size);

/** *****************************************************************
* Permet a un processus de s'attacher a une zone de memoire partagee
*          --> Modifier le type de retour en fonction <--
*          --> du type de ce que l'on veut manipuler <--
****************************************************************** **/
struct memoirePartagee * attachement(int shm_id);

/** 
* Detache le processus courant de la zone de memoire 
* partagee pointee
**/
int detachement(void * shm_addr);

/**
* Detruit la zone de memoire partage pointee
**/
int destruction(int shm_id);

/**
* Cree un tableau de semaphores
* de taille nb_sem
**/
int creaSem(key_t clef, int nb_Sem);

/**
* Initialise un semaphore avec la valeur
* de val_Init
**/
void init_sem(int sem_id,int sem_index, int val_Init, union sem_union controle);

/**
* \param tabOp = tableau de structures sembuf contenant les operations
* \param index_sem = l'index du semaphore sur lequel appliquer cette operation
* \param index_op = l'index de l'operation dans le tableau d'operations
* \param op = l'operation a effectuer (+1,+12,-1...)
**/
void init_op(struct sembuf * tabOp, int index_sem,int index_Op, int op);

/**
* Effectue le nombre d'operations indique par nb_op,
* en partant de la case tabOp[index_op],
* sur le tableau de semaphores indique par sem_id
**/
void operation(int sem_id, struct sembuf * tabOp,int index_op, int nb_op);

/**
* Affiche le tableau de semaphores 
* identifie par sem_id
**/
void semaShow(int sem_id, union sem_union controle, unsigned short * sem_val, int nb_sem);

/**
* verifie que le port passe en parametres 
* appartient a la plage des ports acceptables
* retourne ce port
**/
int verification_Port(char* port);

/**
* retourne une addresse utilisable par les fonctions
* reseau, à partir d'une adresse IP ou d'un nom de site, et d'un port
**/
struct sockaddr creerAdresse( char * nom_serveur, u_short port );

/**
* creer et lie une socket de domaine domaine(AF_INET == IPV4)
* dont le type est soit SOCK_DGRAM(UDP), soit SOCK_STREAM(TCP)
* et dont le port est soit 0 (renvoie en realite un port acceptable libre aleatoire)
* soit un port bien defini, retourne le descripteur de socket
**/
int creerSocket(int domaine, int type, int port);

/**
* Affiche l'adresse IP et le port de l'adresse
* passee en parametre
**/
void showAddress(struct sockaddr * address);
 
/**
* receptionne les message via la socket locale
* caster le type du message en char * et inversement
* \return -1: erreur 0: socket fermee 1: tout s'est bien passe
**/
int receptionTCP(int socket,char * message);

/**
* Envoie message depuis via la socket locale
* caster le type du message en char * et inversement
* \return -1: erreur 0: socket fermee 1: tout s'est bien passe
**/
int envoieTCP(int socket, char * message);

/** 
 * Permet d'effectuer un fgets et d'en supprimer 
 * le retour chariot.
 */
void saisieClavier(char * string);


#endif
