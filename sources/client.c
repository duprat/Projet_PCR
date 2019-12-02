#include "common.h"

int dernierMessage = 0;
char monPseudo[N];
char monID[N];
int terminaison = 1;
struct message memoire[50] ;


void affichage(){
    for( int i = 0; i < dernierMessage; i++){
        affichageMessage2(&memoire[i],monPseudo);
    }
}

void * reception(void * param){
    struct message * messageRecu;
    int * socket = (int *) param;
    int retourTCP = 0;
    
    while(1){
        messageRecu = malloc(sizeof(struct message));
        char ID[N];
        
        /**
        * Reception d'un message
        **/
        retourTCP = receptionTCP(*socket,(char*) messageRecu);
        
        strcpy(ID,&messageRecu->pseudo[20]);
        /**
        * Gestion d'erreur
        **/
        if(retourTCP == 0 || retourTCP == -1){
            exit(EXIT_FAILURE);
        }
        
        /**
        *   Traitement du message recu
        **/
        /*if(strcmp(ID,monID)==0){
            printf("\nMon message \"%s\" a bien été enregistré.\n",messageRecu->text);
            printf("\nEntrez votre message -> \n");
        }
        else{
            printf("\n\033[34m════════════════════════ ✉ ════════════════════════\n");
            printf("\033[00m");
            affichageMessage(messageRecu);
            printf("\033[34m════════════════════════ ✉ ════════════════════════");
            printf("\033[00m");
            printf("\nIl reste %d messages non lus.\n",(messageRecu->nbMessages - dernierMessage));
            affichageMessage(messageRecu);
            printf("\nEntrez votre message -> \n");
        }*/
        
        if( dernierMessage > 50 ){
            for( int i = 0; i < dernierMessage-1; i++ ){
                strcpy(memoire[i].text,memoire[i+1].text);
                strcpy(memoire[i].pseudo,memoire[i+1].pseudo);
                memoire[i].nbMessages = memoire[i+1].nbMessages;
            }
            dernierMessage--;
        }
        strcpy(memoire[dernierMessage].text,messageRecu->text);
        strcpy(memoire[dernierMessage].pseudo,messageRecu->pseudo);
        memoire[dernierMessage].nbMessages = messageRecu->nbMessages;
        dernierMessage++;
        system("clear");
        affichage();
        printf("\nEntrez votre message\n");
        free(messageRecu);
    }
    pthread_exit(NULL);
}

void handler(int sig) 
{ 
    printf("Caught signal %d\n", sig);
    exit(0);
} 

int main(int argc, char** argv) {
    
    pthread_t * threadReception = malloc(sizeof(pthread_t));
    int socket_locale = 0;
    int port_Serveur = 0;
    int retourTCP = 0;
    char nom_serveur[N];
    struct message * messageEnvoie;
    struct sockaddr * adresse_Serveur = malloc(sizeof(struct sockaddr));
    socklen_t longueurAdresse = (socklen_t) sizeof(struct sockaddr);
    
    /**
    * Enregistrement du nom du programme
    **/
    strcpy(NOM_PRGRM, argv[0]);
    
    /**
    * Initialisation du nom du serveur(adresse IP)
    **/
    if(argv[1] != NULL){
        strcpy(nom_serveur,argv[1]);
    }
    else{
        fprintf(stderr,"%s:%s:%d: Il faut donner l'adresse du serveur.\n",NOM_PRGRM,__FILE__,__LINE__);
		exit(EXIT_FAILURE);
    }
    
    /**
    * Initialisation du port du serveur
    **/
    if(argv[2] != NULL){
        port_Serveur = verification_Port(argv[2]);
    }
    else{
        fprintf(stderr,"%s:%s:%d: Il faut donner le port du serveur.\n",NOM_PRGRM,__FILE__,__LINE__);
		exit(EXIT_FAILURE);
    }
    
    /**
    * Initialisation du pseudo du serveur
    **/
    if(argv[3] != NULL){
        if(strlen(argv[3]) > 0 && strlen(argv[3]) < 20){
            strcpy(monPseudo,argv[3]);
        }
        else{
            fprintf(stderr,"%s:%s:%d: Veuillez donner un pseudo qui fait entre 1 et 20 caractères.\n",NOM_PRGRM,__FILE__,__LINE__);
        }
    }
    else{
        fprintf(stderr,"%s:%s:%d: Veuillez donner votre pseudo.\n",NOM_PRGRM,__FILE__,__LINE__);
		exit(EXIT_FAILURE);
    }
    
    socket_locale = creerSocket(AF_INET,SOCK_STREAM,0);
    
    /**
    * Recuperation de l'adresse du serveur
    **/
    *adresse_Serveur = creerAdresse(nom_serveur, (u_short)port_Serveur);
    
    /**
    * Demande de connection au serveur
    **/
    errno = 0;
    connect(socket_locale,adresse_Serveur,longueurAdresse);
    if(errno != 0){
        perror("ERROR CONNECT ");
        exit(EXIT_FAILURE);
    }
    
    /** ******************************** Etape Handshake ****************************************** **/
    messageEnvoie = malloc(sizeof(struct message));
    strcpy(messageEnvoie->pseudo,monPseudo);
    strcpy(messageEnvoie->text,"handshake");
    
    retourTCP = envoieTCP(socket_locale,(char*) messageEnvoie);
    if(retourTCP != 1){
        exit(EXIT_FAILURE);
    }
    
    struct message * messageRecu = malloc(sizeof(struct message));
    retourTCP = receptionTCP(socket_locale,(char*) messageRecu);
    
    strcpy(monPseudo,messageRecu->pseudo);
    strcpy(monID,&monPseudo[20]);

    free(messageRecu);
    /** ******************************* Fin Etape Handshake ****************************************** **/
    
    /**
    * Lancement du thread d'ecoute
    **/
    pthread_create(threadReception,NULL,reception,(void*) &socket_locale);
    
    /** 
    * Partie saisie et envoi d'un message
    **/

   //signal(SIGINT,handler);  

    printf("\nEntrez votre message -> ");
    while( terminaison ){
        messageEnvoie = malloc(sizeof(struct message));
        strcpy(messageEnvoie->pseudo,monPseudo);
        saisieClavier(messageEnvoie->text);
        system("clear");
        messageEnvoie->nbMessages = dernierMessage;
        
        /**
        * Envoie d'un message
        **/
        retourTCP = envoieTCP(socket_locale,(char*) messageEnvoie);
        free(messageEnvoie);
    }
    
    close(socket_locale);
    free(adresse_Serveur);
    free(threadReception);
}
