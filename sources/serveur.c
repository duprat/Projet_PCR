#include <sys/socket.h>
#include <sys/types.h> 

#include "common.h"

#define MEMOIRE 2 


int main(int argc, char* argv[]){

    socklen_t longueurAdresse = (socklen_t) sizeof(struct sockaddr);

    key_t keyMemoire = getKey(MEMOIRE);

    int idMemoire = init_shm(keyMemoire,sizeof(struct memoirePartagee));

    struct memoirePartagee *memoire = attachement(idMemoire);

    int dSock = socket(PF_INET,SOCK_DGRAM,0);

    if( dSock == -1 ){
        perror("Erreur dSock.");
        return -1;
    }

    struct sockaddr_in ad;
    ad.sin_family = AF_INET;
    ad.sin_addr.s_addr = INADDR_ANY;
    ad.sin_port = htons((short)5000);

    int res = bind(dSock, (struct sockaddr*)&ad,sizeof(ad));

    while ( 1 ){

        struct sockaddr_in adresseClient;
        
        if( listen(res,10) == -1 ){
            perror("Erreur listen Serveur.\n");
            return -1;
        }

        if ( accept(dSock,&adresseClient,&longueurAdresse) == -1 ){
            perror("Erreur accept Serveur.\n");
            return -1;
        }

    }

    destruction(idMemoire);

    return 0;
}
