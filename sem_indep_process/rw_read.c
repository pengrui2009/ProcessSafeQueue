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
   
    sem_t * reader = sem_open("readers", O_CREAT, 0600, 1);
    sem_t * writer = sem_open("writer", O_CREAT, 0600, 1);
    sem_t * mut = sem_open("mut", O_CREAT, 0600, 1);

    int count;

    // lock(mutex)
    sem_wait(mut);

    sem_post(reader); //read_count++

    sem_getvalue(reader, &count);
    if(count==1){
        sem_wait(writer);
    }

    sem_post(mut);
    
    // reading is done
    char ch[100];
    scanf("%s",ch);
    printf("I'm reading");

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