#include "common.h"
#include <signal.h>

#define MEMOIRE 10
#define SEMAPHORE 1

struct memoirePartagee * memoire;
struct infosClient * listeClients;
int compteurClient = 0;
int positionClient = 0;
int idSem = 0;
int end = 0;



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
    rendreMemoire.sem_op = 1;
    rendreMemoire.sem_flg = 0;
    
    semop(idSem,&rendreMemoire,1);
}

/**
 * Envoie a partir de la position donnée en paramètre tous les messages manquant au client
 **/
int envoieManquant(int position,struct infosClient * client){
    printf("ENVOIE MANQUANT debut\n");
    printf("Position du client = %d || Position de la mémoire = %d \n",positionClient,memoire->nbPosition);
    int retourTCP = 1;
    struct memoirePartagee * memoireTmp = memoire;
    struct message * message_Envoi;

    for( int i = 0;  position != memoireTmp->position; i++ ){ //Se place a l'emplacement du dernier message connu du client
        memoireTmp = memoireTmp->suivant;
        printf("envoie manquant memoire suivante %d\n",i);
    }

    for( int i = position; i < (memoire->nbPosition); i++){ // Envoie chaque message non connu du client
        printf("Envoie du message numero = %d\n",memoireTmp->position);
        message_Envoi = memoireTmp->commentaire;
        retourTCP = envoieTCP(client->socketClient,(char *)message_Envoi);
        memoireTmp = memoireTmp->suivant;
    }
    if( retourTCP != 1 ){
        fprintf(stderr,"%s:%s:%d: ERROR ENVOI_TCP.\n",NOM_PRGRM,__FILE__,__LINE__);
        if(retourTCP == 0){
            fprintf(stderr,"%s:%s:%d: SOCKET CLOSED.\n",NOM_PRGRM,__FILE__,__LINE__);
            return -1;
        }
    }
    return 0;
}

void affichageMemoire(){
    struct memoirePartagee * memoireTmp = memoire;

    printf(" -------------------------\n");
    printf("| DEBUT AFFICHAGE MEMOIRE |\n");
    printf(" -------------------------\n");
    printf("MEMOIRE NBPOSITION = %d \n",memoire->nbPosition);
    for( int i = 0; i < memoire->nbPosition; i++ ){
        printf("[%s] %s || position message = %d\n",memoireTmp->commentaire->pseudo,memoireTmp->commentaire->text,memoireTmp->position);
        memoireTmp = memoireTmp->suivant;
    }
    printf(" -----------------------\n");
    printf("| FIN AFFICHAGE MEMOIRE |\n");
    printf(" -----------------------\n");
}

void * reception(void * param){
    struct infosClient * clientCourant = param;
    struct message * message_Recu;
    struct message * message_Envoi;
    int retourTCP = 0;

    while(1){
        
        message_Recu = malloc(sizeof(struct message));
        message_Envoi = malloc(sizeof(struct message));

        retourTCP = receptionTCP(clientCourant->socketClient, (char*)message_Recu);
        
         if( retourTCP != 1 ){
              fprintf(stderr,"%s:%s:%d: ERROR RECEPTION_TCP.\n",NOM_PRGRM,__FILE__,__LINE__);
              if(retourTCP == 0){
                  fprintf(stderr,"%s:%s:%d: SOCKET CLOSED.\n",NOM_PRGRM,__FILE__,__LINE__);
                  
                  if(clientCourant->posClient == 0){
                      free(clientCourant->client_suivant);
                  }
                  else{
                      struct infosClient * client_precedent = listeClients;
                      for(int i = 0; i < clientCourant->posClient - 1;i++){
                          client_precedent = client_precedent->client_suivant;
                      }
                      client_precedent->client_suivant = client_precedent->client_suivant->client_suivant;
                  }
                  free(message_Recu);
                  free(message_Envoi);
                  free(clientCourant);
                  compteurClient--;
                  pthread_exit(NULL);
              }
         }
         for(int i = 0; message_Recu->pseudo[i] != ' ';i++){
             printf("%c",message_Recu->pseudo[i]);
         }
         printf(" a dit: %s\n",message_Recu->text);
        /** ********************************* Modif mémoire partagée ****************************************** **/

        printf("DEBUT RECEPTION\n");
        prendreTicket();

        printf(" memoir position = %d | message numero = %d\n",memoire->nbPosition,message_Recu->numero);
        if( message_Recu->numero <= memoire->nbPosition ){ //Si la position du message suposé par le client n'est pas la bonne alors il envoie toute les données manquantes
            envoieManquant(message_Recu->numero-1,clientCourant);
            printf("\n\n\n\nJE RENTRE PAS PLZZ ??\n\n\n\n");
        }

        if( memoire->nbPosition != 0 ){
            printf("\n\npremier element memoire\n");
            printf("commentaire -> %s | position -> %d\n\n\n",memoire->commentaire->text,memoire->position);
        }

        struct memoirePartagee * memoireTmp = memoire;


        for(int i = 0; i < memoire->nbPosition - 1 ; i++ ){ //j'accède au dernier message de la mémoire partagé
            memoireTmp = memoireTmp->suivant;
        }



        if( memoire->nbPosition == 0 ){
            memoire->nbPosition = message_Recu->numero; // Normalement 1 
            memoire->position = 0;
            memoire->commentaire = message_Recu;
            printf("MEMOIRE PREMIER ELEMENT PREMIER COUT\n");
            printf("commentaire -> %s | position -> %d\n\n\n",memoire->commentaire->text,memoire->position);
        }else{
            struct memoirePartagee * nouvelleMemoir = malloc(sizeof(struct memoirePartagee)); //je crée un nouvel element de mémoire partagé pour le dernier message recu
            message_Recu->numero = memoire->nbPosition + 1;
            nouvelleMemoir->commentaire = message_Recu;
            nouvelleMemoir->position = memoire->nbPosition; 
            memoireTmp->suivant = nouvelleMemoir;
            memoire->nbPosition = message_Recu->numero;
            positionClient = message_Recu->numero;
            printf("\n\n\naffichage element courant\n");
            printf("commenatire -> %s | position -> %d\n\n\n",memoireTmp->commentaire->text,memoireTmp->position);
            printf("affichage nouvelle_memoire\n");
            printf("commentaire -> %s | position -> %d \n\n\n",nouvelleMemoir->commentaire->text,nouvelleMemoir->position);
        }


        
        message_Envoi = message_Recu;
        positionClient = message_Recu->numero;
        rendreTicket();
        printf("FIN RECEPTION\n");
        //affichageMemoire();

        /** ******************************* Fin Modif mémoire partagée ****************************************** **/

        retourTCP = envoieTCP(clientCourant->socketClient, (char*)message_Envoi);
        
        if( retourTCP != 1 ){
             fprintf(stderr,"%s:%s:%d: ERROR RECEPTION_TCP.\n",NOM_PRGRM,__FILE__,__LINE__);
             if(retourTCP == 0){
                 fprintf(stderr,"%s:%s:%d: SOCKET CLOSED.\n",NOM_PRGRM,__FILE__,__LINE__);
                 
                 if(clientCourant->posClient == 0){
                     free(clientCourant->client_suivant);
                 }
                 else{
                     struct infosClient * client_precedent = listeClients;
                     for(int i = 0; i < clientCourant->posClient - 1;i++){
                         client_precedent = client_precedent->client_suivant;
                     }
                     client_precedent->client_suivant = client_precedent->client_suivant->client_suivant;
                 }
                 free(message_Recu);
                 free(message_Envoi);
                 free(clientCourant);
                 compteurClient--;
                 pthread_exit(NULL);
             }
        }

        free(message_Recu);
        free(message_Envoi);
    }

    free(clientCourant);
    pthread_exit(NULL);
}


/**
 * Fonction principal de chaque fils 
 **/
int fils(struct infosClient * monClient){

    pthread_t thread;

    struct infosClient * clientCourant = monClient;
    struct message * message_Recu = malloc(sizeof(struct message));
    struct message * message_Envoi = malloc(sizeof(struct message));
    char * ID = NULL;
    int i = 0;
    int retourTCP = 1;
    
    /** ******************************** Etape Handshake ****************************************** **/

    retourTCP = receptionTCP(clientCourant->socketClient,(char*) message_Recu);
    if( retourTCP != 1 ){
        fprintf(stderr,"%s:%s:%d: ERROR RECEPTION_TCP.\n",NOM_PRGRM,__FILE__,__LINE__);
        if(retourTCP == 0){
            fprintf(stderr,"%s:%s:%d: SOCKET CLOSED.\n",NOM_PRGRM,__FILE__,__LINE__);
            return -1;
        }
    }
    
    // attribution ID unique (utilisation de la socket du client)
    while(message_Recu->pseudo[i] != '\0'){ // cherche la fin du pseudo
        i++;
    }
    for(int j = i;j<20;j++){ // remplit les trous avec des espaces
        message_Recu->pseudo[j] = ' ';
    }
    ID = &(message_Recu->pseudo[20]); // decalage case 20
    sprintf(ID, "%d", clientCourant->socketClient); // int to char
    strcpy(message_Envoi->pseudo,message_Recu->pseudo);

    retourTCP = envoieTCP(clientCourant->socketClient, (char*)message_Envoi);

    if( retourTCP != 1 ){
        fprintf(stderr,"%s:%s:%d: ERROR ENVOI_TCP.\n",NOM_PRGRM,__FILE__,__LINE__);
        if(retourTCP == 0){
            fprintf(stderr,"%s:%s:%d: SOCKET CLOSED.\n",NOM_PRGRM,__FILE__,__LINE__);
            return -1;
        }
    }
    /** ******************************* Fin Etape Handshake ****************************************** **/

    pthread_create(&thread,NULL,reception,(void *)clientCourant); //Lancement du thread de reception de message

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
    
    socklen_t longueurAdresse = (socklen_t) sizeof(struct sockaddr);

    key_t keyMemoire = getKey(MEMOIRE);

    int idMemoire = init_shm(keyMemoire,sizeof(struct memoirePartagee));

    memoire = attachement(idMemoire);

    memoire = malloc(sizeof(struct memoirePartagee));

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

    while( end != 1 ){
        //char continuer  = 'O';
        printf("[SERVEUR] Attend de connexion d'un client.\n");
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



        if( fork() == 0 ){ // nouveaux fils car nouveaux client lui donnant au passage son client
            fils(tempClient);
        }
    

        compteurClient++;
        printf("Nombre de client total = %d.\n",compteurClient);
        /*printf("Voulez vous continuez ? O / N \n");
        scanf("%c",&continuer);
        if( continuer == 'N' || continuer == 'n'){
            end = -1;
        }*/
    }
    
    //detachement(memoire);
    close(socket_locale);
    destruction(idMemoire);
    //destruction(idSem);

    return 0;
}
