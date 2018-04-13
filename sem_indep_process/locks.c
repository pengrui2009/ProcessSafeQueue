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
#include <string.h>

#include <fcntl.h>           /* For O_* constants */
#include "locks.h"



void get_read_lock(struct lock_custom *lock){
	
	int count;

	sem_wait(lock->mut); 		// lock(mutex)
    sem_post(lock->reader); 	//read_count++
    
    sem_getvalue(lock->reader, &count);
    if(count==1){
        sem_wait(lock->writer);
    }

    sem_post(lock->mut);
}


void realease_read_lock(struct lock_custom *lock){
	
	int count;

	sem_wait(lock->mut); 		// lock(mutex)
    sem_wait(lock->reader); 	//read_count--
    
    sem_getvalue(lock->reader, &count);
    if(count==0){
        sem_post(lock->writer);
    }
    
    sem_post(lock->mut);
}


void get_write_lock(struct lock_custom *lock) {
	sem_wait(lock->writer);
}

void realease_write_lock(struct lock_custom *lock) {
	sem_post(lock->writer);
}

void print_lock(struct lock_custom *lock){

    int count_mut;
    sem_getvalue(lock->mut, &count_mut);
    printf("mut: %d\n",count_mut);

    int count_reader;
    sem_getvalue(lock->reader, &count_reader);
    printf("reader: %d\n",count_reader);
    
    int count_writer;
    sem_getvalue(lock->writer, &count_writer);
    printf("writer: %d\n",count_writer);

}


struct lock_custom *init_lock_custom(struct lock_custom *lock, char *id) {
	
	lock = (struct lock_custom *) malloc(sizeof(struct lock_custom *));

	char readers[100] = "readers";
	lock->reader = sem_open(strcat(readers, id), O_CREAT, 0600, 1);

	char writer[100] = "writer";
    lock->writer = sem_open(strcat(writer,id), O_CREAT, 0600, 1);

    char mut[100] = "mut";
    lock->mut = sem_open(strcat(mut,id), O_CREAT, 0600, 1);

    // sem_post(lock->writer);
    // sem_post(lock->mut);
    sem_wait(lock->reader);

    return lock;
}




struct lame_queue *init_queue(int id){
	
	int shm_id;
    key_t key = id;
    struct lame_queue *queue;

    // Creating a segment
    if ((shm_id = shmget(key, sizeof(struct lame_queue), IPC_CREAT | 0666)) < 0) {
        perror("shm_get error");
        exit(1);
    }

    // shm is the link the shared mem
    if ((queue = (struct lame_queue *) shmat(shm_id, NULL, 0)) ==  -1) {
        perror("error in shmat");
        exit(1);
    }

    // queue = (struct lame_queue *)malloc(sizeof(struct lame_queue));

    queue->start_index = 0;
    queue->end_index = 0;

    /*
        Setting up queue mutex
    */
    char enqueue_muttex_str[100];
    sprintf(enqueue_muttex_str, "enqueue_muttex%d", id);
    queue->enqueue_muttex = sem_open(enqueue_muttex_str, O_CREAT, 0600, 1);
    // sem_post(queue->enqueue_muttex);

    char dequeue_muttex_str[100];
    sprintf(dequeue_muttex_str, "dequeue_muttex%d", id);
    queue->dequeue_muttex = sem_open(dequeue_muttex_str, O_CREAT, 0600, 1);
    // sem_post(queue->dequeue_muttex);

    for (int i = 0; i < MAX_CAPACITY; ++i) {
        char idd[50];
        sprintf(idd, "q%delement%d", id,i);
        queue->array[i].lock = *init_lock_custom(&queue->array[i].lock,  idd);
    	queue->array->data = (char *) malloc(1024*sizeof(char));
    }
    

    return queue;
}

struct lame_queue *sync_queue(int id, struct lame_queue *queue) {
	int shm_id;
    key_t key = id;

    // Creating a segment
    if ((shm_id = shmget(key, sizeof(struct lame_queue), IPC_CREAT | 0666)) < 0) {
        perror("shm_get error lol");
        exit(1);
    }

    // shm is the link the shared mem
    if ((queue = (struct lame_queue *) shmat(shm_id, NULL, 0)) ==  -1) {
        perror("error in shmat");
        exit(1);
    }

    return queue;
}

void queue_read_index(struct lame_queue *queue, int index) {
    get_read_lock(&queue->array[index].lock);
    printf("Index: %d \t Content: %s\n", index, queue->array[index].data);
    realease_read_lock(&queue->array[index].lock);
}

void queue_write_index(struct lame_queue *queue, int index, char *str){
    get_write_lock(&queue->array[index].lock);
    strcpy(queue->array[index].data, str);
    realease_write_lock(&queue->array[index].lock);
}

void queue_enqueue(struct lame_queue *queue, char *str){
    sem_wait(queue->enqueue_muttex); 
    get_write_lock(&queue->array[queue->end_index].lock);
    strcpy(queue->array[queue->end_index++].data, str);
    realease_write_lock(&queue->array[queue->end_index - 1].lock);
    sem_post(queue->enqueue_muttex); 
}

char * queue_dequeue(struct lame_queue *queue){
    char *str = (char *) malloc(1024*sizeof(char));
    sem_wait(queue->dequeue_muttex); 
    get_write_lock(&queue->array[queue->start_index].lock);
    strcpy(str,queue->array[queue->start_index].data);
    queue->start_index++;
    realease_write_lock(&queue->array[queue->start_index - 1].lock);
    sem_post(queue->dequeue_muttex); 
    return str;
}

int main(int argc, char const *argv[]){
	struct lame_queue *queue;
    queue = sync_queue(1234, queue);
    // get_read_lock(&queue->array[1].lock);
    // print_lock(&queue->array[1].lock);
    // realease_read_lock(&queue->array[1].lock);
    // print_lock(&queue->array[2].lock);
    queue->array[2].data = "sup";
    // char x[100] ="what up";
    queue_enqueue(queue, "sup");

	return 0;
}