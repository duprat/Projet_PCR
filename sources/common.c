#include "common.h"

key_t getKey(int myInt){
    key_t myKey;
    // Recuperation de la clef
    errno = 0;
    myKey = ftok(".",myInt);

    if( errno != 0){
        perror("Error FTOK ");
        exit(EXIT_FAILURE);
    }
    return myKey;
}

int init_shm(key_t key, size_t size){
    
    int shm_id = 0;

    // Creation ou obtention de l'adresse de l'espace memoire partage
    errno = 0;
    shm_id = shmget(key, size, IPC_CREAT|0666);

    if( errno != 0){
        perror("Error SHMGET ");
        exit(EXIT_FAILURE);
    }
    
    return shm_id;
}

struct memoirePartagee * attachement(int shm_id){
    struct memoirePartagee * shm_addr = NULL;
    errno = 0;
    shm_addr = shmat(shm_id, NULL, 0);

    if( errno != 0){
        perror("Error SHMAT ");
        exit(EXIT_FAILURE);
    }

    return shm_addr;
}

int detachement(void * shm_addr){
    errno = 0;
    shmdt(shm_addr);

    if( errno != 0){
        perror("Error SHMDT ");
        exit(EXIT_FAILURE);
    }
    return 0;
}

int destruction(int shm_id){
    errno = 0;
    shmctl(shm_id, IPC_RMID, NULL);

    if( errno != 0){
        perror("Error SHMCTL ");
        exit(EXIT_FAILURE);
    }
    return 0;
}

void init_op(struct sembuf * tabOp, int numSem,int numOp, int op){
    tabOp[numOp].sem_num = numSem;
    tabOp[numOp].sem_op = op;
    tabOp[numOp].sem_flg = 0;
}

/**
* Creation du tableau de semaphores
**/
int creaSem(key_t clef, int nb_Sem){
    int sem_id;
    errno = 0;
    sem_id = semget(clef,nb_Sem,IPC_CREAT|0666);
    if( errno != 0){
        perror("Error SEMGET ");
        exit(EXIT_FAILURE);
    }
    return sem_id;
}

void init_sem(int sem_id,int sem_index, int val_Init, union sem_union controle){
    /**
    * Initialisation de l'union permettant le controle
    * sur le semaphore
    **/
    controle.val = val_Init;
    
    /**
    * Initialisation du semaphore sem_index
    **/
    errno = 0;
    semctl(sem_id, sem_index, SETVAL, controle);
    if( errno != 0){
        perror("Error SEM_INIT ");
        exit(EXIT_FAILURE);
    }
}


void operation(int sem_id, struct sembuf * tabOp, int index_op, int nb_op){
    errno = 0;
    semop(sem_id,&tabOp[index_op],nb_op);
    if( errno != 0){
        perror("Error SEMOP ");
        exit(EXIT_FAILURE);
    }
}


void semaShow(int sem_id, union sem_union controle, unsigned short * sem_val, int nb_sem){
    controle.array = sem_val;
    errno = 0;
    semctl(sem_id,0,GETALL,controle);
    if( errno != 0){
        perror("Error SEM_CTL GETALL ");
        exit(EXIT_FAILURE);
    }
    
    int cptr = 1;
    for(int i = 0;i<nb_sem;i++,cptr++){
        printf("sem[%d]=%d\t",i,sem_val[i]);
        if(cptr == 4){
            printf("\n");
            cptr = 0;
        }
    }
    printf("\n");
}

int verification_Port(char* port){

	if(atoi(port)<1025 || atoi(port)>65535){
		printf("\033[31m");
		fprintf(stderr,"%s:%s:%d: Le port n'appartient pas à la plage [1025;65535].\n",NOM_PRGRM,__FILE__,__LINE__);
		exit(EXIT_FAILURE);
	}
	return atoi(port);
}

struct sockaddr creerAdresse( char * nom_serveur, u_short port )
{
	struct sockaddr_in tempAddr;
	struct sockaddr addrRetour;
	struct hostent *retourGetHostByName ;
	u_long IP;
    
    memset((char*)&tempAddr, 0, sizeof(tempAddr));
    /*	Soit on fournit une adresse IP, soit on fournit un nom	*/
    if ((IP = inet_addr(nom_serveur)) == (u_long)INADDR_NONE)
	{
	    if ((retourGetHostByName  = gethostbyname(nom_serveur))==NULL)
	    {
            memset( (char * )&addrRetour, 0, sizeof(addrRetour) );
    	    return  addrRetour;
	    }
	    tempAddr.sin_family = retourGetHostByName->h_addrtype;

    	memcpy (&tempAddr.sin_addr, retourGetHostByName->h_addr, retourGetHostByName->h_length);

    }
    else
	{
        tempAddr.sin_family = AF_INET;
        tempAddr.sin_addr.s_addr = IP;
    }

    /*  Port destination    */    
    tempAddr.sin_port = htons((u_short)port );    

    memcpy( (char *)&addrRetour, (char *)&tempAddr, sizeof(addrRetour) );
    return addrRetour;
}

 int creerSocket(int domaine, int type, int port){
	
	static struct sockaddr_in addr_local;
	int sock;

    errno = 0;
    // domaine,type,protocole
	sock=socket(domaine,type,0);
	if(sock==-1){
		perror("ERROR SOCKET ");
		exit(EXIT_FAILURE);
	}

	/* Concerne l'@ local de la socket crée précedemment */
    errno = 0;
	bzero((char*)&addr_local,sizeof(struct sockaddr_in));
	if(errno != 0){
	    perror("ERRO BZERO ");
	    exit(EXIT_FAILURE);
	}

	addr_local.sin_family=domaine;
	addr_local.sin_port=htons(port);
	addr_local.sin_addr.s_addr=htonl(INADDR_ANY);

    errno = 0;
	if(bind(sock,(struct sockaddr*)&addr_local,sizeof(addr_local))==-1){
		perror("ERROR BIND ");
		exit(EXIT_FAILURE);
	}
	return sock;
}

void showAddress(struct sockaddr * address){
    struct sockaddr_in * showcase;
    showcase = (struct sockaddr_in*) address;
    printf("IP: [%s] Port: [%d]",inet_ntoa(showcase->sin_addr),ntohs(showcase->sin_port));
}

/**
* receptionne les message en provenance de socket
* le type du message est a modifier en fonction de ce qui est à recevoir
* retourne le nombre d'octets recus au total
**/
/*long int receptionTCP(int socket,struct message * message, long int taille){
    int index = 0;
    do{
        errno = 0;
        index += recv(socket,&message[index],taille,0);
        if(errno != 0){
            perror(" ERROR RECV ");
            exit(EXIT_FAILURE);
        }
    }while(index < taille);
    return index;
}*/

/**
* receptionne les message en provenance de socket
* caster le type du message en char * et inversement
* \return -1: erreur 0: socket fermee 1: tout s'est bien passe
**/
int receptionTCP(int socket,char * message){
    int index = 0;
    long int taille = sizeof(message);
    do{
        int tempRCV = 0;
        taille -= index;
        tempRCV = recv(socket,&message[index],taille,0);
        if(tempRCV == 0){
            fprintf(stderr,"%s:%s:%d: SOCKET FERMEE.\n",NOM_PRGRM,__FILE__,__LINE__);
		    return 0;
        }
        if(tempRCV == -1){
            fprintf(stderr,"%s:%s:%d: ERROR RECEPTION_TCP.\n",NOM_PRGRM,__FILE__,__LINE__);
            return -1;
        }
        index += tempRCV;
    }while(index < taille);
    return 1;
}

/**
* Envoie message vers socket
* caster le type du message en char * et inversement
* \return -1: erreur 0: socket fermee 1: tout s'est bien passe
**/
int envoieTCP(int socket, char * message){
    int index = 0;
    long int taille = sizeof(message);
    do{
        int tempRCV = 0;
        taille -= index;
        index += send(socket,&message[index], taille, 0);
        if(tempRCV == 0){
            fprintf(stderr,"%s:%s:%d: SOCKET FERMEE.\n",NOM_PRGRM,__FILE__,__LINE__);
		    return 0;
        }
        if(tempRCV == -1){
            fprintf(stderr,"%s:%s:%d: ERROR ENVOI_TCP.\n",NOM_PRGRM,__FILE__,__LINE__);
            return -1;
        }
        index += tempRCV;
    }while(index < taille);
    return 1;
}

void saisieClavier(char * string){
    int i = 0;
    strcpy(string,"");
    fgets(string,N,stdin);
    while(string[i] != '\n'){
        i++;
    }
    string[i] = '\0';
}

