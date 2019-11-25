#include "common.h"

#define MEMOIRE 2 

struct infosClient * listeClients;
int compteurClient;

void * threadService(void * param){
    
    pthread_exit(NULL);
}

int main(int argc, char* argv[]){

    int socket_locale = 0;
    int port_Serveur = 0;
    int socket_client = 0;
    

    compteurClient = 0;
    
    socklen_t longueurAdresse = (socklen_t) sizeof(struct sockaddr);

    key_t keyMemoire = getKey(MEMOIRE);

    int idMemoire = init_shm(keyMemoire,sizeof(struct memoirePartagee));

    struct memoirePartagee *memoire = attachement(idMemoire);
    
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
        listeClients[compteurClient] = malloc(sizeof(struct listeClients));
        listeClients[compteurClient].position = compteurClient;
        listeClients[compteurClient].adresseClient = tempAddr;
        listeClients[compteurClient].socketClient = socket_client;
        pthread_create(&thread,NULL,threadService,NULL);


        compteurClient++;
    }
    
    close(socket_locale);
    destruction(idMemoire);

    return 0;
}
