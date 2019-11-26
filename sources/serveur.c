#include <sys/socket.h>
#include <sys/types.h> 

#include "common.h"

#define MEMOIRE 10
#define SEMAPHORE 1

struct memoirePartagee * memoire;
struct infosClient * listeClients;
int compteurClient;
int positionClient = 0;
int idSem = 0;

/**
 * prend un ticket pour l'accès a la mémoire
 **/
void prendreTicket(){

    struct sembuf accesMemoire;
    accesMemoire.sem_num = 0;
    accesMemoire.sem_op = -1;
    accesMemoire.sem_flg = 0;

    semop(idSem,&accesMemoire,1);
}


/**
 * rend un ticket pour l'accès a la mémoire
 **/
void rendreTicket(){

    struct sembuf rendreMemoire;
    rendreMemoire.sem_num = 0;
    rendreMemoire.sem_op = -1;
    rendreMemoire.sem_flg = 0;

    semop(idSem,&rendreMemoire,1);

}

/**
 * Envoie a partir de la position donnée en paramètre tout les messages manquant au client
 **/
void envoieManquant(int position,struct infosClient client){
    int retourTCP = 1;
    struct memoirePartagee * memoireTmp = memoire;

    for( int i = 0; i != position; i++ ){ //Ce place a l'emplacement du dernier message connu du client
        memoireTmp = memoireTmp->suivant;
    }

    for( int i = position; i < memoire->nbPosition; i++){ // Envoie chaque message non connu du client
        retourTCP = envoieTCP(client.socketClient,(char *)memoireTmp->commentaire);
        memoireTmp = memoireTmp->suivant;
    }

    if( retourTCP != 1 ){
        printf(" Erreur retourTCP dans envoie Manquant serveur ! \n ");
    }

}

void * reception(void * param){
    struct infosClient * clientCourant = param;
    struct message * message_Recu = malloc(sizeof(struct message));
    struct message * message_Envoi = malloc(sizeof(struct message));
    int retourTCP;

    while(1){
        free(message_Recu);
        free(message_Envoi);
        message_Recu = malloc(sizeof(struct message));
        message_Envoi = malloc(sizeof(struct message));

        retourTCP = receptionTCP(clientCourant->socketClient, (char*)message_Recu);

        /** ********************************* Modif mémoire partagée ****************************************** **/

        prendreTicket();

        int position = message_Recu->numero;
        if( position < memoire->nbPosition ){ //Si la position du message suposé par le client n'est pas la bonne alors il envoie toute les données manquantes
            envoieManquant(position-1,*clientCourant);
        }

        struct memoirePartagee * memoireTmp = memoire;

        for(int i = 0; i < memoire->nbPosition; i++ ){ //j'accède au dernier message de la mémoire partagé
            memoireTmp = memoireTmp->suivant;
        }

        struct memoirePartagee * nouvelleMemoir = malloc(sizeof(struct memoirePartagee)); //je crée un nouvelle element de mémoire partagé pour le dernier message recu

        message_Recu->numero = memoire->nbPosition + 1;

        nouvelleMemoir->commentaire = message_Recu;
        nouvelleMemoir->position = message_Recu->numero;

        memoireTmp->suivant = nouvelleMemoir;

        memoire->nbPosition = message_Recu->numero;

        message_Envoi = message_Recu;
        rendreTicket();
        /** ******************************* Fin Modif mémoire partagée ****************************************** **/

        retourTCP = envoieTCP(clientCourant->socketClient, (char*)message_Envoi);

        if( retourTCP != 1 ){
            printf(" erreur envoieTCP dans reception serveur ! \n");
        }
    }

    pthread_exit(NULL);
}


/**
 * Fonction principal de chaque fils 
 **/
void fils(struct infosClient monClient){

    pthread_t thread;

    struct infosClient  clientCourant = monClient;
    struct message * message_Recu = malloc(sizeof(struct message));
    struct message * message_Envoi = malloc(sizeof(struct message));
    char * ID = NULL;
    int i = 0;
    int retourTCP = 1;

    /** ******************************** Etape Handshake ****************************************** **/

    retourTCP = receptionTCP(clientCourant.socketClient, (char*)message_Recu);

    // attribution ID unique (utilisation de la socket du client)
    while(message_Recu->pseudo[i] != '\0'){ // cherche la fin du pseudo
        i++;
    }
    for(int j = i;j<20;j++){ // remplit les trous avec des espaces
        message_Recu->pseudo[j] = ' ';
    }
    ID = &(message_Recu->pseudo[20]); // decalage case 20
    sprintf(ID, "%d", clientCourant.socketClient); // int to char
    strcpy(message_Envoi->pseudo,message_Recu->pseudo);

    retourTCP = envoieTCP(clientCourant.socketClient, (char*)message_Envoi);

    if( retourTCP != 1 ){
        printf(" Erreur retourTCP dans fils serveur ! \n");
    }

    /** ******************************* Fin Etape Handshake ****************************************** **/

    pthread_create(&thread,NULL,reception,(void *)&clientCourant); //Lancement du thread de reception de message

    //Boucle qui vérifie si le client est bien à jour
    while ( 1 ) 
    {
        prendreTicket();
        if( memoire->nbPosition != positionClient){
            envoieManquant(positionClient,clientCourant);
        }
        rendreTicket();
    }
    
}

int main(int argc, char* argv[]){

    int socket_locale = 0;
    int port_Serveur = 0;
    int socket_client = 0;

    listeClients = malloc(sizeof(struct infosClient));

    compteurClient = 0;
    
    socklen_t longueurAdresse = (socklen_t) sizeof(struct sockaddr);

    key_t keyMemoire = getKey(MEMOIRE);

    int idMemoire = init_shm(keyMemoire,sizeof(struct memoirePartagee));

    memoire = attachement(idMemoire);

    key_t keySem = getKey(SEMAPHORE);

    idSem = creaSem(keySem,1);

    union sem_union sem;

    init_sem(idSem,0,1,sem);
    
    strcpy(NOM_PRGRM, argv[0]);
    
    /**
    * mise au format du port serveur
    **/
    if(argv[1] != NULL){
        port_Serveur = verification_Port(argv[1]);
    }
    else{
        fprintf(stderr,"%s:%s:%d: Il faut donner le port du serveur.\n",NOM_PRGRM,__FILE__,__LINE__);
		exit(EXIT_FAILURE);
    }

    /**
    * création de la socket_locale avec le port passe en parametre
    **/
    socket_locale = creerSocket(AF_INET,SOCK_STREAM,port_Serveur);

    while(1){
        struct sockaddr tempAddr;
        struct infosClient * tempClient = listeClients;
        /**
        * Attente de demande de connection
        **/
        errno = 0;
        listen(socket_locale,10);
        if(errno != 0){
            perror(" ERROR LISTEN ");
            exit(EXIT_FAILURE);
        }
        
        /**
        * Creation du tunnel virtuel
        **/
        errno = 0;
        socket_client = accept(socket_locale,&tempAddr,&longueurAdresse);
        if(errno != 0){
            perror(" ERROR ACCEPT ");
            exit(EXIT_FAILURE);
        }

        for(int i = 0; i<compteurClient;i++){
            tempClient = tempClient->client_suivant;
        }

        tempClient->posClient = compteurClient;
        tempClient->adresseClient = tempAddr;
        tempClient->socketClient = socket_client;
        tempClient->client_suivant = malloc(sizeof(struct infosClient));
        
        if( fork() == 0 ){
            fils(*tempClient); // nouveaux fils car nouveaux client lui donnant au passage son client
        }
        
        compteurClient++;
    }
    
    close(socket_locale);
    destruction(idMemoire);
    destruction(idSem);

    return 0;
}
