#include "common.h"

#define MEMOIRE 2 

struct memoirePartagee * memoire;
struct infosClient * listeClients;
int compteurClient;

void * threadService(void * param){
    struct infosClient * clientCourant = param;
    struct message * message_Recu = malloc(sizeof(struct message));
    struct message * message_Envoi = malloc(sizeof(struct message));
    char * ID = NULL;
    /** ******************************** Etape Handshake ****************************************** **/

    retourTCP = receptionTCP(clientCourant->socketClient, (char*)message_Recu);

    // attribution ID unique (utilisation de la socket du client)
    ID = &(message_Recu->pseudo[20]); // decalage case 20
    sprintf(ID, "%d", message_Recu->socket_client); // int to char

    message_Envoi->pseudo = message_Recu->pseudo;

    retourTCP = envoieTCP(clientCourant->socketClient, (char*)message_Envoi);

    /** ******************************* Fin Etape Handshake ****************************************** **/

    while(1){
        free(message_Recu);
        free(message_Envoi)
        message_Recu = malloc(sizeof(struct message));
        message_Envoi = malloc(sizeof(struct message));

        retourTCP = receptionTCP(clientCourant->socketClient, (char*)message_Recu);

        /** ********************************* Modif mémoire partagée ****************************************** **/



        /** ******************************* Fin Modif mémoire partagée ****************************************** **/

        retourTCP = receptionTCP(clientCourant->socketClient, (char*)message_Envoi);
    }

    pthread_exit(NULL);
}

int main(int argc, char* argv[]){

    int socket_locale = 0;
    int port_Serveur = 0;
    int socket_client = 0;
    
    listeClients = malloc(sizeof(struct listeClients));

    compteurClient = 0;
    
    socklen_t longueurAdresse = (socklen_t) sizeof(struct sockaddr);

    key_t keyMemoire = getKey(MEMOIRE);

    int idMemoire = init_shm(keyMemoire,sizeof(struct memoirePartagee));

    memoire = attachement(idMemoire);
    
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
        pthread_t thread;
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
            tempClient = tempClient.client_suivant;
        }

        tempClient->position = compteurClient;
        tempClient->adresseClient = tempAddr;
        tempClient->socketClient = socket_client;
        tempClient->client_suivant = malloc(sizeof(struct infosClient));

        pthread_create(&thread,NULL,threadService,(void *)&tempClient);


        compteurClient++;
    }
    
    close(socket_locale);
    destruction(idMemoire);

    return 0;
}
