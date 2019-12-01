#include "common.h"
#include <signal.h>

#define MEMOIRE 10
#define SEMAPHORE 1


int numeroMessage = 0;
int idSem = 0;
int end = 0;
int idMemoire = 0;
int idListeClient = 0;


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



/** ****************************************** AFFICHAGE ******************************************** **/

void affichageMemoire(struct memoirePartagee * memoire){
    struct memoirePartagee * memoireTmp = memoire;

    printf("\n\n -------------------------\n");
    printf("| DEBUT AFFICHAGE MEMOIRE |\n");
    printf(" -------------------------\n");
    printf("MEMOIRE nbMessages = %d \n",memoire->nbMessages);
    while(memoireTmp->suivant != NULL){
        printf("[%s] %s || numeroMessage = %d\n",memoireTmp->commentaire.pseudo,memoireTmp->commentaire.text,memoireTmp->numeroMessage);
        memoireTmp = memoireTmp->suivant;
    }
    printf("[%s] %s || numeroMessage = %d\n",memoireTmp->commentaire.pseudo,memoireTmp->commentaire.text,memoireTmp->numeroMessage);
    printf(" -----------------------\n");
    printf("| FIN AFFICHAGE MEMOIRE |\n");
    printf(" -----------------------\n\n");
}

void affichageClients(struct infosClient * liste){
    struct infosClient * listeTmp = liste;

    printf("\n\n ------------------------------\n");
    printf("| DEBUT AFFICHAGE LISTE_CLIENT |\n");
    printf(" ------------------------------\n");
    printf("LISTECLIENT nbClients = %d \n",liste->nbClients);
    for(int i = 0;i<liste->nbClients;i++){
        printf("socketClient= %d || posClient = %d\n",listeTmp->socketClient,listeTmp->posClient);
        listeTmp = listeTmp->client_suivant;
    }
    printf(" ----------------------------\n");
    printf("| FIN AFFICHAGE LISTE_CLIENT |\n");
    printf(" ----------------------------\n\n");
}

/** ****************************************** FIN AFFICHAGE ******************************************** **/

/**
 * Envoie a partir de la position donnée en paramètre tous les messages manquant au client
 **/
int envoieManquant(int numeroMessageClient, struct infosClient * client,struct infosClient * listeClients, struct memoirePartagee * memoire){
    int retourTCP = 1;
    struct memoirePartagee * memoireTmp = memoire;
    struct message * message_Envoi = malloc(sizeof(struct message));
    
    /** ************ numeroMessage = message_Recu->numero - 1 ****************** **/
    
    printf("ENVOIE MANQUANT debut\n");
    printf("Numero du message du client = %d || numeroMessage de la mémoire = %d \n",numeroMessageClient,memoire->nbMessages);
    
    /**
    * On se place a l'endroit ou le client pensait se mettre 
    **/
    for( int i = 0; i < numeroMessageClient ; i++ ){ 
        memoireTmp = memoireTmp->suivant;
        printf("envoie manquant memoire suivante %d\n",i);
    }
    
    if(memoire->nbMessages != 0){
         if(numeroMessageClient == memoire->nbMessages){
             strcpy(message_Envoi->pseudo,memoireTmp->commentaire.pseudo);
             strcpy(message_Envoi->text,memoireTmp->commentaire.text);
             message_Envoi->numero = memoireTmp->commentaire.numero;
             
             retourTCP = envoieTCP(client->socketClient,(char *)message_Envoi);
             if( retourTCP != 1 ){
                 fprintf(stderr,"%s:%s:%d: ERROR ENVOI_TCP.\n",NOM_PRGRM,__FILE__,__LINE__);
                 if(retourTCP == 0){
                     fprintf(stderr,"%s:%s:%d: SOCKET CLOSED.\n",NOM_PRGRM,__FILE__,__LINE__);
                     return -1;
                 }
             }
             numeroMessage++;
         }
         else{
             for( int i = numeroMessageClient; i < memoire->nbMessages; i++){ // Envoie chaque message non connu du client
                 printf("Envoie du message numero = %d\n",memoireTmp->numeroMessage);
                 printf("numeroMessageClient= %d\n",numeroMessageClient);
                 
                 strcpy(message_Envoi->pseudo,memoireTmp->commentaire.pseudo);
                 strcpy(message_Envoi->text,memoireTmp->commentaire.text);
                 message_Envoi->numero = numeroMessageClient++;
                 
                 retourTCP = envoieTCP(client->socketClient,(char *)message_Envoi);
                 if( retourTCP != 1 ){
                     fprintf(stderr,"%s:%s:%d: ERROR ENVOI_TCP.\n",NOM_PRGRM,__FILE__,__LINE__);
                     if(retourTCP == 0){
                         fprintf(stderr,"%s:%s:%d: SOCKET CLOSED.\n",NOM_PRGRM,__FILE__,__LINE__);
                         return -1;
                     }
                 }
                 memoireTmp = memoireTmp->suivant;
                 numeroMessage++;
             }
         }
    }
    else{
        printf("Aucun message en memoire.\n");
    }
    free(message_Envoi);
    printf("SORTIE Envoie Manaquant\n");
    return 0;
}

void * reception(void * param){
    struct paramThread * paramThread = param;
    struct message * message_Recu;
    struct message * message_Envoi;
    struct memoirePartagee * memoireTmp = paramThread->memoire;
    int retourTCP = 0;

    while(1){
        
        printf("\n\nDEBUT RECEPTION\n");
        
        message_Recu = malloc(sizeof(struct message));
        message_Envoi = malloc(sizeof(struct message));
        
        /**
        *   Reception de message
        **/
        retourTCP = receptionTCP(paramThread->client_courant->socketClient, (char*)message_Recu);
        
        /**
        * Gestion erreurs
        **/
        if( retourTCP != 1 ){
            fprintf(stderr,"%s:%s:%d: ERROR RECEPTION_TCP.\n",NOM_PRGRM,__FILE__,__LINE__);
            if(retourTCP == 0){
                fprintf(stderr,"%s:%s:%d: SOCKET CLOSED.\n",NOM_PRGRM,__FILE__,__LINE__);

                if(paramThread->client_courant->posClient == 0){
                    free(paramThread->client_courant->client_suivant);
                }
                else{
                    struct infosClient * client_precedent = paramThread->premier_client;
                    for(int i = 0; i < paramThread->client_courant->posClient - 1;i++){
                    client_precedent = client_precedent->client_suivant;
                    }
                    client_precedent->client_suivant = client_precedent->client_suivant->client_suivant;
                }
                free(message_Recu);
                free(message_Envoi);
                free(paramThread->client_courant);
                paramThread->premier_client->nbClients--;
                pthread_exit(NULL);
            }
        }
         
         /** 
         *  affichage du message et de son envoyeur
         **/ 
         for(int i = 0; message_Recu->pseudo[i] != ' ';i++){
             printf("%c",message_Recu->pseudo[i]);
         }
         printf(" a dit: %s\n",message_Recu->text);
         printf("numero auquel le client compte placer ce message= %d\n",message_Recu->numero);
         
        /** ********************************* Modif mémoire partagée ****************************************** **/
        
        prendreTicket();
        printf("\nDEBUT ECRITURE\n");
        
        printf("nombre de messages dans la memoire= %d | numero auquel le client compte placer son message: %d\n",paramThread->memoire->nbMessages,message_Recu->numero);
        
        if( paramThread->memoire->nbMessages == 0 ){
            printf("CAS DU PREMIER MESSAGE\n");
            paramThread->memoire->nbMessages++; // Normalement 1 
            paramThread->memoire->numeroMessage = message_Recu->numero; // peut etre 0
            
            paramThread->memoire->commentaire.numero = message_Recu->numero;
            paramThread->memoire->commentaire.dernierMessage = message_Recu->numero;
            
            strcpy(paramThread->memoire->commentaire.text,message_Recu->text);
            strcpy(paramThread->memoire->commentaire.pseudo,message_Recu->pseudo);

        }else{
            printf("CAS D'UN MESSAGE CLASSIQUE\n");
            while(memoireTmp->suivant != NULL){ //j'accède au dernier message de la mémoire partagé
                memoireTmp = memoireTmp->suivant;
            }
            
            memoireTmp->suivant = shmat(idMemoire, NULL, 0);
                                  if( errno != 0){
                                      perror("Error SHMAT ");
                                      exit(EXIT_FAILURE);
                                  }//(malloc(sizeof(struct memoirePartagee)); //je crée un nouvel element de mémoire partagé
                                                                           //  pour le dernier message recu
            memoireTmp = memoireTmp->suivant;
            paramThread->memoire->nbMessages++;
            message_Recu->numero = paramThread->memoire->nbMessages;
            
            strcpy(memoireTmp->commentaire.pseudo,message_Recu->pseudo);
            strcpy(memoireTmp->commentaire.text,message_Recu->text);
            
            memoireTmp->numeroMessage = paramThread->memoire->nbMessages;
            memoireTmp->suivant = NULL;
        }

        affichageMemoire(paramThread->memoire);
        
        message_Envoi->dernierMessage = paramThread->memoire->nbMessages;
        message_Envoi->numero = paramThread->memoire->nbMessages;
        strcpy(message_Envoi->pseudo,memoireTmp->commentaire.pseudo);
        strcpy(message_Envoi->text,memoireTmp->commentaire.text);
        
        numeroMessage = paramThread->memoire->nbMessages;
        
        affichageMemoire(paramThread->memoire);
        rendreTicket();
        printf("FIN ECRITURE\n\n");

        /** ******************************* Fin Modif mémoire partagée ****************************************** **/

        retourTCP = envoieTCP(paramThread->client_courant->socketClient, (char*)message_Envoi);
        
        if( retourTCP != 1 ){
             fprintf(stderr,"%s:%s:%d: ERROR RECEPTION_TCP.\n",NOM_PRGRM,__FILE__,__LINE__);
             if(retourTCP == 0){
                 fprintf(stderr,"%s:%s:%d: SOCKET CLOSED.\n",NOM_PRGRM,__FILE__,__LINE__);
                 
                 if(paramThread->client_courant->posClient == 0){
                     free(paramThread->client_courant->client_suivant);
                 }
                 else{
                     struct infosClient * client_precedent = paramThread->premier_client;
                     for(int i = 0; i < paramThread->client_courant->posClient - 1;i++){
                         client_precedent = client_precedent->client_suivant;
                     }
                     client_precedent->client_suivant = client_precedent->client_suivant->client_suivant;
                 }
                 free(message_Recu);
                 free(message_Envoi);
                 free(paramThread->client_courant);
                 paramThread->premier_client->nbClients--;
                 pthread_exit(NULL);
             }
        }
       /* if( message_Recu->numero < paramThread->memoire->nbMessages ){ //Si la position du message supposé par le client n'est pas la bonne
                                                                        //alors il envoie toute les données manquantes
            printf("envoieManquant a la reception d'un message\n");
            envoieManquant(message_Recu->numero,paramThread->client_courant,paramThread->premier_client,paramThread->memoire);
            printf("\n\n\n\nJE RENTRE PAS PLZZ ??\n\n\n\n");
        }*/
        
        free(message_Recu);
        free(message_Envoi);
    }

    free(paramThread->client_courant);
    pthread_exit(NULL);
}


/**
 * Fonction principal de chaque fils 
 **/
int fils(struct infosClient * listeClients, struct memoirePartagee * memoire, struct infosClient * monClient){
    affichageMemoire(memoire);
    affichageClients(listeClients);
    pthread_t thread;
    struct paramThread * paramThread = malloc(sizeof(struct paramThread));
    struct message * message_Recu = malloc(sizeof(struct message));
    struct message * message_Envoi = malloc(sizeof(struct message));
    struct memoirePartagee * memoireTmp = memoire;
    char * ID = NULL;
    int i = 0;
    int retourTCP = 1;
    
    
    /** ******************************** Etape Handshake ****************************************** **/

    retourTCP = receptionTCP(monClient->socketClient,(char*) message_Recu);
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
    sprintf(ID, "%d", monClient->socketClient); // int to char
    strcpy(message_Envoi->pseudo,message_Recu->pseudo);
    
    printf("[SERVEUR] Je lui donne le pseudo \"%s\"\n",message_Envoi->pseudo);
    
    retourTCP = envoieTCP(monClient->socketClient, (char*)message_Envoi);
    
    if( retourTCP != 1 ){
        fprintf(stderr,"%s:%s:%d: ERROR ENVOI_TCP.\n",NOM_PRGRM,__FILE__,__LINE__);
        if(retourTCP == 0){
            fprintf(stderr,"%s:%s:%d: SOCKET CLOSED.\n",NOM_PRGRM,__FILE__,__LINE__);
            return -1;
        }
    }
    
    printf("[SERVEUR] La conversation comporte %d messages\n",memoire->nbMessages);
    
    affichageMemoire(memoire);
    
    prendreTicket();
    message_Envoi->dernierMessage = memoire->nbMessages;
    
    if(memoire->nbMessages != 0){
        printf("Recupration messages handshake\n");
        envoieManquant(message_Recu->numero,monClient,listeClients, memoire);
    }

    rendreTicket();
    
    /** ******************************* Fin Etape Handshake ****************************************** **/
    
    paramThread->memoire = memoire;
    paramThread->premier_client = listeClients;
    paramThread->client_courant = monClient;
    
    pthread_create(&thread,NULL,reception,(void *)paramThread); //Lancement du thread de reception de message

    //Boucle qui vérifie si le client est bien à jour
    while ( 1 ) 
    {   
        
        if(memoire->nbMessages != 0){
            prendreTicket();
            if( memoire->nbMessages > numeroMessage){
                printf("Boucle qui vérifie si le client est bien à jour\n");
                envoieManquant(numeroMessage,monClient,listeClients,memoire);
            }
            rendreTicket();
            //printf("ticket rendu\n");
        }
        
    }
    free(monClient);
}


/** *********************************************************************
 *                              MAIN                                    *
 ********************************************************************* **/

int main(int argc, char* argv[]){
    
    int socket_locale = 0;
    int port_Serveur = 0;
    int socket_client = 0;
    struct memoirePartagee memoire = NULL;
    struct infosClient * listeClients = NULL;
    socklen_t longueurAdresse = (socklen_t) sizeof(struct sockaddr);

    key_t keyMemoire = getKey(MEMOIRE);
    key_t keyClients = getKey((MEMOIRE+1));
    key_t keySem = getKey(SEMAPHORE);
    
    idSem = creaSem(keySem,1);
    
    union sem_union sem;
    
    init_sem(idSem,0,1,sem);

    idMemoire = init_shm(keyMemoire,sizeof(struct memoirePartagee));
    idListeClient = init_shm(keyClients,sizeof(struct infosClient));
    
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

    listeClients->nbClients = 0;
    memoire->nbMessages = 0;
    
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
            
            struct infosClient * tempClient = listeClients;
            
            while(tempClient->client_suivant != NULL){
                tempClient = tempClient->client_suivant;
            }
            
            listeClients->nbClients++;
            
            tempClient->posClient = listeClients->nbClients;
            tempClient->adresseClient = tempAddr;
            tempClient->socketClient = socket_client;
            tempClient->client_suivant = malloc(sizeof(struct infosClient));
            
            /*printf("Voulez vous continuez ? O / N \n");
            scanf("%c",&continuer);
            if( continuer == 'N' || continuer == 'n'){
            end = -1;
            }*/
            fils(listeClients, memoire, tempClient);
        }
    }
        
    
    close(socket_locale);
    destruction(idMemoire);
    //destruction(idSem);

    return 0;
}
