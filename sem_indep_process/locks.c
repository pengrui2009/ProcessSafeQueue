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

#define MAX_CAPACITY 3

struct lock_custom {
	sem_t * reader;
	sem_t * writer;
	sem_t * mut;
};


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


struct lock_custom *init_lock_custom(struct lock_custom *lock, char *id) {
	
	lock = (struct lock_custom *) malloc(sizeof(struct lock_custom));

	char readers[100] = "readers";
	lock->reader = sem_open(strcat(readers, id), O_CREAT, 0600, 1);

	char writer[100] = "writer";
    lock->writer = sem_open(strcat(writer,id), O_CREAT, 0600, 1);

    char mut[100] = "mut";
    lock->mut = sem_open(strcat(mut,id), O_CREAT, 0600, 1);
}


struct element {
	char *data;
	struct lock_custom lock;
};

struct lame_queue {
	int start_index;
	int end_index;
	struct element array[MAX_CAPACITY];
};

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

//     queue = (struct lame_queue *)malloc(sizeof(struct lame_queue));

    queue->start_index = 0;
    queue->end_index = 0;

    for (int i = 0; i < MAX_CAPACITY; ++i) {
        char idd[50];
        sprintf(idd, "%d",i);
        init_lock_custom(&queue->array[i].lock,  idd);
    	queue->array->data = (char *) malloc(sizeof(char));
    }
    

    return queue;
}

// struct lame_queue *sync(int id){
	
// 	int shm_id;
//     key_t key = id;
//     struct lame_queue *queue;

//     // Creating a segment
//     if ((shm_id = shmget(key, sizeof(struct lame_queue), IPC_CREAT | 0666)) < 0) {
//         perror("shm_get error");
//         exit(1);
//     }

//     // shm is the link the shared mem
//     if ((queue = (struct lame_queue *) shmat(shm_id, NULL, 0)) ==  -1) {
//         perror("error in shmat");
//         exit(1);
//     }

//     return queue;
// }



int main(int argc, char const *argv[]){
	struct lame_queue *queue = init_queue(123);
    get_read_lock(&queue->array[1].lock);
	return 0;
}