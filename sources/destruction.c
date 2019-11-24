#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <stdlib.h>

int main( int argc, char* argv[]){
    
    key_t cleSemaphore = ftok(".",1);

    if( cleSemaphore == -1 ){
        perror("Erreur clé Semaphore\n");
        return -1;
    }

    int idSemaphore = semget(cleSemaphore,0,0666);

    if( idSemaphore == -1 ){
        perror("Erreur id Sémaphore\n");
        return -1;
    }

    if( semctl(idSemaphore,0,IPC_RMID) == -1 ){
        perror("Erreur destruction Sémaphore.\n");
        return -1;
    }

    key_t cleSegment = ftok(".",2);

    if( cleSegment == -1 ){
        perror("Erreur clé Segement\n");
        return -1;
    }

    int idSegment = shmget(cleSegment,sizeof(int),IPC_CREAT|0666);

    if( idSegment == -1 ){
        perror("Erreur id Segement\b");
        return -1;
    }

    if( shmctl(idSegment,IPC_RMID,NULL) == -1 ){
        perror("Erreur destruction Mémoire partagées?\n");
    }


    return 0;
}