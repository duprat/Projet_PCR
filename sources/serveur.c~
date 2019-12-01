#include "common.h"
#include <signal.h>

#define MEMOIRE 10
#define SEMAPHORE 1

int dernierePosClient = 0;
int dernierMessageClient = 0;
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

/** ****************************************** AFFICHAGE ******************************************** **/

void affichageMemoire(struct memoirePartagee * memoire){
    if(memoire[0].nbMessages != 0){
        printf("\n\n -------------------------\n");
        printf("| DEBUT AFFICHAGE MEMOIRE |\n");
        printf(" -------------------------\n");
        printf("MEMOIRE nbMessages = %d \n",memoire[0].nbMessages);
        for(int i = 0;i<memoire[0].nbMessages;i++){
            printf("[%s] %s || indexMessage = %d\n",memoire[i].commentaire.pseudo,memoire[i].commentaire.text,i);
        }
        printf(" -----------------------\n");
        printf("| FIN AFFICHAGE MEMOIRE |\n");
        printf(" -----------------------\n\n");
    }
    
}

void affichageClients(struct infosClient * liste){

    printf("\n\n ------------------------------\n");
    printf("| DEBUT AFFICHAGE LISTE_CLIENT |\n");
    printf(" ------------------------------\n");
    printf("LISTECLIENT nbClients = %d \n",liste[0].nbClients);
    for(int i = 0;i<liste[0].nbClients;i++){
        printf("socketClient= %d || posClient = %d\n",liste[i].socketClient,i);
    }
    printf(" ----------------------------\n");
    printf("| FIN AFFICHAGE LISTE_CLIENT |\n");
    printf(" ----------------------------\n\n");
}

/** 
 *  affichage du message et de son envoyeur
 **/ 
void affichageMessage(struct message * message){
    
    for(int i = 0; message->pseudo[i] != ' ';i++){
        printf("%c",message->pseudo[i]);
    }
    printf(" a dit: %s\n",message->text);
}

/** ****************************************** FIN AFFICHAGE ******************************************** **/

/**
 * Envoie a partir de la position donnée en paramètre tous les messages manquant au client
 **/
int envoieManquant(int nbMessagesClient, int indexClient,struct infosClient * listeClients, struct memoirePartagee * memoire){
    int retourTCP = 1;
    struct message * message_Envoi = malloc(sizeof(struct message));
    
    printf("[%d] ENVOIE MANQUANT debut\n",getpid());
    printf("[%d] nbMessages client = %d || nbMessages mémoire = %d \n",getpid(),nbMessagesClient,memoire[0].nbMessages);
    
    if(memoire[0].nbMessages != 0){
        for( int i = nbMessagesClient; i < memoire[0].nbMessages; i++){ // Envoie chaque message non connu du client
            printf("[%d] Envoie du message numero = %d\n",getpid(),i);

            strcpy(message_Envoi->pseudo,memoire[i].commentaire.pseudo);
            strcpy(message_Envoi->text,memoire[i].commentaire.text);
            
            printf("[%d] Message -> %s \n",getpid(),message_Envoi->text);
            printf("[%d] socket du client -> %d\n",getpid(),listeClients[indexClient].socketClient);

            retourTCP = envoieTCP(listeClients[indexClient].socketClient,(char *)message_Envoi);
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
    printf("[%d] SORTIE Envoie Manaquant\n",getpid());
    return memoire[0].nbMessages;
}

void * reception(void * param){

    struct paramThread * paramThread = param;
    struct infosClient * listeClients = paramThread->listeClients;
    struct memoirePartagee * memoire = paramThread->memoire;
    int indexClientCourant = paramThread->index_client;
    
    struct message * message_Recu;
    struct message * message_Envoi;
    int retourTCP = 0;

    while(1){
        
        printf("\n\n[%d] DEBUT RECEPTION\n",getpid());
        
        message_Recu = malloc(sizeof(struct message));
        message_Envoi = malloc(sizeof(struct message));
        
        /**
        *   Reception de message
        **/
        retourTCP = receptionTCP(listeClients[indexClientCourant].socketClient, (char*)message_Recu);
        if( retourTCP != 1 ){
            fprintf(stderr,"%s:%s:%d: ERROR RECEPTION_TCP.\n",NOM_PRGRM,__FILE__,__LINE__);
            if(retourTCP == 0){
                fprintf(stderr,"%s:%s:%d: SOCKET CLOSED.\n",NOM_PRGRM,__FILE__,__LINE__);
                pthread_exit(NULL);
            }
        }
         
         affichageMessage(message_Recu);
         
        /** ********************************* Modif mémoire partagée ****************************************** **/
        
        prendreTicket();
        printf("\n[%d] DEBUT ECRITURE\n",getpid());
        printf("[%d] nbMessages dans la memoire= %d | nbMessages connus par le client: %d\n",getpid(),memoire[0].nbMessages,message_Recu->nbMessages);
        
        printf("[%d] MODIFICATION MEMOIRE\n",getpid());
        
        memoire[0].nbMessages++;
        strcpy(memoire[(memoire[0].nbMessages - 1)].commentaire.pseudo,message_Recu->pseudo);
        strcpy(memoire[(memoire[0].nbMessages - 1)].commentaire.text,message_Recu->text);
        
        printf("[%d] PREPARATION DONNEES A ENVOYER\n",getpid());
        
        message_Envoi->nbMessages = memoire[0].nbMessages;
        strcpy(message_Envoi->pseudo,memoire[(memoire[0].nbMessages - 1)].commentaire.pseudo);
        strcpy(message_Envoi->text,memoire[(memoire[0].nbMessages - 1)].commentaire.text);
        
        affichageMemoire(memoire);
        
        dernierMessageClient++;
        
        rendreTicket();
        printf("[%d] FIN ECRITURE\n\n",getpid());

        /** ******************************* Fin Modif mémoire partagée ****************************************** **/

        retourTCP = envoieTCP(listeClients[indexClientCourant].socketClient, (char*)message_Envoi);
        
        if( retourTCP != 1 ){
             fprintf(stderr,"%s:%s:%d: ERROR RECEPTION_TCP.\n",NOM_PRGRM,__FILE__,__LINE__);
             if(retourTCP == 0){
                 fprintf(stderr,"%s:%s:%d: SOCKET CLOSED.\n",NOM_PRGRM,__FILE__,__LINE__);
                 pthread_exit(NULL);
             }
        }
        printf("[%d] MESSAGE ENVOYE\n\n",getpid());
        
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
int fils(struct infosClient * listeClients, struct memoirePartagee * memoire, int indexClient){

    pthread_t thread;
    struct paramThread * paramThread = malloc(sizeof(struct paramThread));
    struct message * message_Recu = malloc(sizeof(struct message));
    struct message * message_Envoi = malloc(sizeof(struct message));
    char * ID = NULL;
    int retourTCP = 1;
    dernierMessageClient = 0;
    
    
    /** ******************************** Etape Handshake ****************************************** **/

    retourTCP = receptionTCP(listeClients[indexClient].socketClient,(char*) message_Recu);
    if( retourTCP != 1 ){
        fprintf(stderr,"%s:%s:%d: ERROR RECEPTION_TCP.\n",NOM_PRGRM,__FILE__,__LINE__);
        if(retourTCP == 0){
            fprintf(stderr,"%s:%s:%d: SOCKET CLOSED.\n",NOM_PRGRM,__FILE__,__LINE__);
            return -1;
        }
    }
    
    ID = attributionID(message_Recu,message_Envoi,listeClients[indexClient]);
    
    printf("[SERVEUR] Je lui donne le pseudo \"%s\"\n",message_Envoi->pseudo);
    
    retourTCP = envoieTCP(listeClients[indexClient].socketClient, (char*)message_Envoi);
    
    if( retourTCP != 1 ){
        fprintf(stderr,"%s:%s:%d: ERROR ENVOI_TCP.\n",NOM_PRGRM,__FILE__,__LINE__);
        if(retourTCP == 0){
            fprintf(stderr,"%s:%s:%d: SOCKET CLOSED.\n",NOM_PRGRM,__FILE__,__LINE__);
            return -1;
        }
    }
    
    prendreTicket();
    
    affichageMemoire(memoire);
    message_Envoi->nbMessages = memoire[0].nbMessages;
    
    if(memoire[0].nbMessages != 0){
        printf("[%d] Recuperation messages handshake\n",getpid());
        dernierMessageClient =  envoieManquant(dernierMessageClient,indexClient,listeClients, memoire);
    }

    rendreTicket();
    
    /** ******************************* Fin Etape Handshake ****************************************** **/
    
    paramThread->memoire = memoire;
    paramThread->listeClients = listeClients;
    paramThread->index_client = indexClient;
    
    pthread_create(&thread,NULL,reception,(void *)paramThread); //Lancement du thread de reception de message

    /**
    * Boucle qui vérifie si le client est bien à jour
    **/
    while ( 1 ) 
    {
        if(memoire[0].nbMessages != 0){
            prendreTicket();
            if( memoire[0].nbMessages > dernierMessageClient){
                printf("[%d] DERNIER MESSAGE CLIENT  = %d | nbMessageMemoire = %d\n",getpid(),dernierMessageClient,memoire[0].nbMessages);
                printf("[%d] Boucle qui vérifie si le client est bien à jour\n",getpid());
                dernierMessageClient =  envoieManquant(dernierMessageClient,indexClient,listeClients,memoire);
            }
            rendreTicket();
        }
    }
}


/** *********************************************************************
 *                              MAIN                                    *
 ********************************************************************* **/

int main(int argc, char* argv[]){
    
    int socket_locale = 0;
    int port_Serveur = 0;
    int socket_client = 0;
    struct memoirePartagee * memoire = NULL;
    struct infosClient * listeClients = NULL;
    socklen_t longueurAdresse = (socklen_t) sizeof(struct sockaddr);

    key_t keyMemoire = getKey(MEMOIRE);
    key_t keyClients = getKey((MEMOIRE+1));
    key_t keySem = getKey(SEMAPHORE);
    
    idSem = creaSem(keySem,1);
    
    union sem_union sem;
    
    init_sem(idSem,0,1,sem);

    int idMemoire = init_shm(keyMemoire,sizeof(struct memoirePartagee)*N);
    int idListeClient = init_shm(keyClients,sizeof(struct infosClient)*N);
    
    /**
    *   Attachement a la memoire partagee et a la liste de clients
    **/
    listeClients = shmat(idListeClient, NULL, 0);
    if( errno != 0){
        perror("Error SHMAT ");
        exit(EXIT_FAILURE);
    }
    
    memoire = shmat(idMemoire, NULL, 0);
    if( errno != 0){
        perror("Error SHMAT ");
        exit(EXIT_FAILURE);
    }
    
    /**
    *   Initialisation valeurs
    **/
    for(int i = 0; i<N;i++){
        listeClients[i].nbClients = 0;
        listeClients[i].socketClient = 0;
        memoire[i].nbMessages = 0;
    }
    
    detachement(memoire);
    detachement(listeClients);

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
            struct infosClient * listeClients = NULL;
            
           
            /**
            *   Attachement a la memoire partagee et a la liste de clients
            **/
            listeClients = shmat(idListeClient, NULL, 0);
            if( errno != 0){
                perror("Error SHMAT ");
                exit(EXIT_FAILURE);
            }
            
            memoire = shmat(idMemoire, NULL, 0);
            if( errno != 0){
                perror("Error SHMAT ");
                exit(EXIT_FAILURE);
            }

            listeClients[listeClients[0].nbClients].adresseClient = tempAddr;
            listeClients[listeClients[0].nbClients].socketClient = socket_client;
            
            /*printf("Voulez vous continuez ? O / N \n");
            scanf("%c",&continuer);
            if( continuer == 'N' || continuer == 'n'){
            end = -1;
            }*/
            
            fils(listeClients, memoire, listeClients[0].nbClients);
            listeClients[0].nbClients++;
        }
    }
        
    
    close(socket_locale);
    destruction(idMemoire);
    //destruction(idSem);

    return 0;
}
