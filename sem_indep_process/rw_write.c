#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>

#define NO_ELEMENTS 600
size_t shm_size = sizeof(char) * NO_ELEMENTS;

int main(int argc, char *argv[]){

    if(argc != 3 ){
        printf("That's too much, man! \n");
    }
    int index = atoi(argv[1]);
    char data = argv[2][0];

   
    sem_t * writer = sem_open("writer", O_CREAT, 0600, 1);

    int shm_id;
    key_t key = 123456;
    char *shm;

    // Creating a segment
    if ((shm_id = shmget(key, shm_size, IPC_CREAT | 0666)) < 0) {
        perror("shm_get error");
        exit(1);
    }

    // shm is the link the shared mem
    if ((shm = shmat(shm_id, NULL, 0)) == (char *) -1) {
        perror("error in shmat");
        exit(1);
    }

    

    sem_wait(writer);

    printf("I'm writing\n");
    shm[index] = data;

    sem_post(writer);

    sem_close(writer);
    return 1;
}