#include "common.h"

#define MEMOIRE 10
#define SEMAPHORE 1

int dernierePosClient = 0;
int dernierMessageClient = 0;
int idSem = 0;
int end = 0;
struct infosClient * ceClient = NULL;
int terminaison = 1;

/**
 * prend un ticket pour l'accès a la mémoire
 **/
void prendreTicket(){
    struct sembuf accesMemoire;
    accesMemoire.sem_num = 0;
    accesMemoire.sem_op = -1;
    accesMemoire.sem_flg = 0;

    if ( semop(idSem,&accesMemoire,1) == -1 ){
        perror("Erreur prendreTciket.\n");
        exit(EXIT_FAILURE);
    }
}


/**
 * rend un ticket pour l'accès a la mémoire
 **/
void rendreTicket(){
    struct sembuf rendreMemoire;
    rendreMemoire.sem_num = 0;
    rendreMemoire.sem_op = 1;
    rendreMemoire.sem_flg = 0;
    
    if( semop(idSem,&rendreMemoire,1) == -1 ){
        perror("Erreur rendreTciket.\n");
        exit(EXIT_FAILURE);
    }
}

/**
 * attribution ID unique avec utilisation de la socket du client
 **/
char * attributionID(struct message * message_Recu, struct message * message_Envoi, struct infosClient client_courant){
    char * ID = NULL;
    int i = 0;
    while(message_Recu->pseudo[i] != '\0'){ // cherche la fin du pseudo
        i++;
    }
    for(int j = i;j<20;j++){ // remplit les trous avec des espaces
        message_Recu->pseudo[j] = ' ';
    }
    ID = &(message_Recu->pseudo[20]); // decalage case 20
    sprintf(ID, "%d", client_courant.socketClient); // int to char
    strcpy(message_Envoi->pseudo,message_Recu->pseudo);
    return ID;
}

/** *********************************************************************
 *                              AFFICHAGE                               *
 ********************************************************************* **/

void affichageMemoire(struct memoirePartagee * memoire){
    //system("clear");
    if(memoire[0].nbMessages != 0){
        printf("\n  -------------------------\n");
        printf("| DEBUT AFFICHAGE MEMOIRE |\n");
        printf(" -------------------------\n");
        for(int i = 0;i<memoire[0].nbMessages;i++){
            affichageMessage2(&memoire[i].commentaire,NULL);
        }
        printf(" -----------------------\n");
        printf("| FIN AFFICHAGE MEMOIRE |\n");
        printf(" -----------------------\n");
    }
    
}

/**
 * Envoie a partir de la position donnée en paramètre tous les messages manquant au client
 **/
int envoieManquant(int nbMessagesClient, struct memoirePartagee * memoire){
    int retourTCP = 1;
    struct message * message_Envoi = malloc(sizeof(struct message));
    
    if(memoire[0].nbMessages != 0){
        for( int i = nbMessagesClient; i < memoire[0].nbMessages; i++){ // Envoie chaque message non connu du client

            strcpy(message_Envoi->pseudo,memoire[i].commentaire.pseudo);
            strcpy(message_Envoi->text,memoire[i].commentaire.text);
            message_Envoi->nbMessages = (memoire[0].nbMessages - 1);
            retourTCP = envoieTCP(ceClient->socketClient,(char *)message_Envoi);
            if( retourTCP != 1 ){
                fprintf(stderr,"%s:%s:%d: ERROR ENVOI_TCP.\n",NOM_PRGRM,__FILE__,__LINE__);
                
                if(retourTCP == 0){
                    fprintf(stderr,"%s:%s:%d: SOCKET CLOSED.\n",NOM_PRGRM,__FILE__,__LINE__);
                    return -1;
                }
            }
        }
    }
    else{
        printf("[%d] Aucun message en memoire.\n",getpid());
    }
    free(message_Envoi);
    dernierMessageClient = memoire[0].nbMessages;
    return memoire[0].nbMessages;
}


/** *********************************************************************
 *                              RECEPTION                               *
 ********************************************************************* **/

void * reception(void * param){

    struct memoirePartagee * memoire = param;
    
    struct message * message_Recu;
    struct message * message_Envoi;
    int retourTCP = 0;

    while(1){
        
        message_Recu = malloc(sizeof(struct message));
        message_Envoi = malloc(sizeof(struct message));
        
        /**
        *   Reception de message
        **/
        retourTCP = receptionTCP(ceClient->socketClient, (char*)message_Recu);
        if( retourTCP != 1 ){
            fprintf(stderr,"%s:%s:%d: ERROR RECEPTION_TCP.\n",NOM_PRGRM,__FILE__,__LINE__);
            if(retourTCP == 0){
                fprintf(stderr,"%s:%s:%d: SOCKET CLOSED.\n",NOM_PRGRM,__FILE__,__LINE__);
                printf("[%d] Client deconnecté.\n",getpid());
                terminaison = 0;
                pthread_exit(NULL);
            }
        }
         
        /** ********************************* Modif mémoire partagée ****************************************** **/
        
        prendreTicket();
        
        memoire[0].nbMessages++;
        strcpy(memoire[(memoire[0].nbMessages - 1)].commentaire.pseudo,message_Recu->pseudo);
        strcpy(memoire[(memoire[0].nbMessages - 1)].commentaire.text,message_Recu->text);
        
        message_Envoi->nbMessages = memoire[0].nbMessages;
        strcpy(message_Envoi->pseudo,memoire[(memoire[0].nbMessages - 1)].commentaire.pseudo);
        strcpy(message_Envoi->text,memoire[(memoire[0].nbMessages - 1)].commentaire.text);
        
        affichageMemoire(memoire);
        
        dernierMessageClient++;
        
        rendreTicket();

        /** ******************************* Fin Modif mémoire partagée ****************************************** **/

        retourTCP = envoieTCP(ceClient->socketClient, (char*)message_Envoi);
        
        if( retourTCP != 1 ){
             fprintf(stderr,"%s:%s:%d: ERROR RECEPTION_TCP.\n",NOM_PRGRM,__FILE__,__LINE__);
             if(retourTCP == 0){
                fprintf(stderr,"%s:%s:%d: SOCKET CLOSED.\n",NOM_PRGRM,__FILE__,__LINE__); 
                printf("[%d] Client deconnecté.\n",getpid());
                terminaison = 0;
                pthread_exit(NULL);
             }
        }
        
        free(message_Recu);
        free(message_Envoi);
    }

    pthread_exit(NULL);
}

/** ******************************************************************************** **/
/**                                   FILS                                           **/
/** ******************************************************************************** **/
/**
 * Fonction principal de chaque fils 
 **/
int fils(struct memoirePartagee * memoire){

    pthread_t thread;
    struct message * message_Recu = malloc(sizeof(struct message));
    struct message * message_Envoi = malloc(sizeof(struct message));
    int retourTCP = 1;
    dernierMessageClient = 0;
    
    
    /** ******************************** Etape Handshake ****************************************** **/

    retourTCP = receptionTCP(ceClient->socketClient,(char*) message_Recu);
    if( retourTCP != 1 ){
        fprintf(stderr,"%s:%s:%d: ERROR RECEPTION_TCP.\n",NOM_PRGRM,__FILE__,__LINE__);
        if(retourTCP == 0){
            fprintf(stderr,"%s:%s:%d: SOCKET CLOSED.\n",NOM_PRGRM,__FILE__,__LINE__);
            return 0;
        }
    }
    
    attributionID(message_Recu,message_Envoi,*ceClient);
    
    retourTCP = envoieTCP(ceClient->socketClient, (char*)message_Envoi);
    
    if( retourTCP != 1 ){
        fprintf(stderr,"%s:%s:%d: ERROR ENVOI_TCP.\n",NOM_PRGRM,__FILE__,__LINE__);
        if(retourTCP == 0){
            fprintf(stderr,"%s:%s:%d: SOCKET CLOSED.\n",NOM_PRGRM,__FILE__,__LINE__);
            return 0;
        }
    }
    
    prendreTicket();
    
    affichageMemoire(memoire);
    message_Envoi->nbMessages = memoire[0].nbMessages;
    
    if(memoire[0].nbMessages != 0){
        dernierMessageClient =  envoieManquant(dernierMessageClient,memoire);
    }

    rendreTicket();
    
    /** ******************************* Fin Etape Handshake ****************************************** **/
    
    affichagePseudo(message_Recu->pseudo);
    printf(" s'est connecté.\n");
    pthread_create(&thread,NULL,reception,(void *)memoire); //Lancement du thread de reception de message

    /**
    * Boucle qui vérifie si le client est bien à jour
    **/
    while ( terminaison ) 
    {
        if(memoire[0].nbMessages != 0){
            prendreTicket();
            if( memoire[0].nbMessages > dernierMessageClient){
                dernierMessageClient =  envoieManquant(dernierMessageClient,memoire);
            }
            rendreTicket();
        }
    }
    return 0;
}


/** *********************************************************************
 *                              MAIN                                    *
 ********************************************************************* **/

int main(int argc, char* argv[]){
    
    int socket_locale = 0;
    int port_Serveur = 0;
    int socket_client = 0;
    struct memoirePartagee * memoire = NULL;
    socklen_t longueurAdresse = (socklen_t) sizeof(struct sockaddr);

    key_t keyMemoire = getKey(MEMOIRE);

    key_t keySem = getKey(SEMAPHORE);
    
    idSem = creaSem(keySem,1);
    
    union sem_union sem;
    
    init_sem(idSem,0,1,sem);

    int idMemoire = init_shm(keyMemoire,sizeof(struct memoirePartagee)*N);
    
    /**
    *   Attachement a la memoire partagee
    **/
    memoire = attachement(idMemoire);
    
    /**
    *   Initialisation valeurs
    **/
    for(int i = 0; i<N;i++){
        memoire[i].nbMessages = 0;
    }
    
    detachement(memoire);

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

    while( terminaison ){
        //char continuer  = 'O';
        printf("[SERVEUR] Attente de connexion d'un client.\n");
        struct sockaddr tempAddr;

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

/** ************************************* FILS ******************************************** **/

        if(fork() == 0){
            struct memoirePartagee * memoire = NULL;
            ceClient = malloc(sizeof(struct infosClient));
            
           ceClient->socketClient = socket_client;
           ceClient->adresseClient = tempAddr;

            /**
            *   Attachement a la memoire partagee pour le fils
            **/
            
            memoire = attachement(idMemoire);
            
            terminaison = fils(memoire);
        }
    }
/** ************************************* FIN FILS **************************************** **/
    
    printf("[%d] terminé TERMINE.\n",getpid());
    if( terminaison ){
        close(socket_locale);
        destruction(idMemoire);
    }

    return 0;
}
