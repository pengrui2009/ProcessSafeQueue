#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/stat.h>

int main(){
   
    sem_t * writer = sem_open("writer", O_CREAT, 0600, 1);


    sem_wait(writer);

    // writing is done
    char ch[100];
    scanf("%s",ch);
    printf("I'm writing");

    sem_post(writer);

    sem_close(writer);
    return 1;
}