#include "common.h"

int position;
char monPseudo[N];

void * reception(void * param){
    struct message * messageRecu;
    int * socket = (int *) param;
    int retourTCP = 0;
    
    while(1){
        messageRecu = malloc(sizeof(struct message));
        
        /**
        * Reception d'un message
        **/
        retourTCP = receptionTCP(*socket,(char*) messageRecu);
        
        /**
        * Gestion d'erreur
        **/
        if(retourTCP == 0 || retourTCP == -1){
            exit(EXIT_FAILURE);
        }
        
        /**
        *   Traitement du message recu
        **/
        if(strcmp(messageRecu->pseudo,monPseudo)==0){
            printf("Mon message a bien été enregistré.\n");
            printf("\"%s\"\n",messageRecu->text);
        }
        else{
            printf("Il reste %d messages non lus.\n",(messageRecu->numero - position));
            printf("%s:\n",messageRecu->pseudo);
            printf("  %s\n",messageRecu->text);
        }
        position++;
        free(messageRecu);
    }
    pthread_exit(NULL);
}


int main(int argc, char** argv) {
    
    pthread_t * threadReception = malloc(sizeof(pthread_t));
    int socket_locale = 0;
    int port_Serveur = 0;
    int retourTCP = 0;
    position = -1;
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
    
    /**
    * Etape de handshake
    **/
    messageEnvoie = malloc(sizeof(struct message));
    strcpy(messageEnvoie->pseudo,monPseudo);
    
    retourTCP = envoieTCP(socket_locale,(char*) messageEnvoie);
    perror(NULL);
    
    struct message * messageRecu = malloc(sizeof(struct message));
    retourTCP = receptionTCP(socket_locale,(char*) messageRecu);
    
    strcpy(monPseudo,messageRecu->pseudo);
    free(messageRecu);
    
    /**
    * Lancement du thread d'ecoute
    **/
    pthread_create(threadReception,NULL,reception,(void*) &socket_locale);
    
    /** 
    * Partie saisie et envoi d'un message
    **/
    while(1){
        messageEnvoie = malloc(sizeof(struct message));
        printf("Entrez votre message.\n");
        saisieClavier(messageEnvoie->text);
        
        printf("Je vais envoyer \"%s\"\n",messageEnvoie->text);
        
        /**
        * Envoie d'un message
        **/
        retourTCP = envoieTCP(socket_locale,(char*) messageEnvoie);
        perror(NULL);
        
        printf("J'ai envoye %d octets.\n\n",retourTCP);
        free(messageEnvoie);
    }
    
    close(socket_locale);
    free(adresse_Serveur);
    free(threadReception);
}

