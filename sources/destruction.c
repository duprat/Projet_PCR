#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <stdlib.h>

int main( int argc, char* argv[]){
    
    int nbSemaphores = 0;
    int nbMemoires = 0;

    int condition = 1;

    int cpt = 1;

    while( condition ){

        key_t cleSemaphore = ftok(".",cpt);

        if( cleSemaphore == -1 ){
            perror("Erreur clé Semaphore\n");
            return -1;
        }

        int idSemaphore = semget(cleSemaphore,0,0666);

        if( idSemaphore == -1 ){
            condition = 0;
        }

        if( semctl(idSemaphore,0,IPC_RMID) == -1 ){
            condition = 0;
        }else{
            nbSemaphores += 1;
            cpt++;
        }
        

    }
    
    condition = 1;

    cpt = 10;

    while ( condition )
    {
        key_t cleSegment = ftok(".",cpt);

        if( cleSegment == -1 ){
            perror("Erreur clé Segement\n");
            return -1;
        }

        int idSegment = shmget(cleSegment,sizeof(int),IPC_CREAT|0666);

        if( idSegment == -1 ){
            condition = 0;
        }

        if( shmctl(idSegment,IPC_RMID,NULL) == -1 ){
            condition = 0;
        }else
        {
            nbMemoires += 1;
            cpt++;
        }
        
    }

    printf("Nombre de semaphore détruite = %d.\n",nbSemaphores);

    printf("Nombre de segment de mémoire dértuit = %d.\n",nbMemoires);
    

    return 0;
}