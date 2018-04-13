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

    int my_weird_case = -1;
    if(argc != 3 ){
        printf("That's too much, man! \n");
    }

    int choice = atoi(argv[1]); // as if I've one
    int  = atoi()

    sem_t * reader = sem_open("readers", O_CREAT, 0600, 1);
    sem_t * writer = sem_open("writer", O_CREAT, 0600, 1);
    sem_t * mut = sem_open("mut", O_CREAT, 0600, 1);

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

    int count;

    // one followed by index
    // two followed by dequeue
    char choice = getc(stdin);
    int index;

    if(choice == '1')
         scanf("%d",&index);
    

    // lock(mutex)
    sem_wait(mut);

    sem_post(reader); //read_count++

    sem_getvalue(reader, &count);
    if(count==1){
        sem_wait(writer);
    }

    sem_post(mut);
    
    // reading is done
    printf("I'm reading\n\n");
    if(choice == '1'){
        printf("Index : %d \n Content: %c",index, shm[index+1]);
    }

    // lock(mutex)
    sem_wait(mut);

    sem_wait(reader); // readcount--

    sem_getvalue(reader, &count);
    if(count==0){
        sem_post(writer);
    }

    sem_post(mut);


    sem_close(reader);
    sem_close(writer);
    sem_close(mut);
    return 1;
}