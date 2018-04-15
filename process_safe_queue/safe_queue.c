#include <stdio.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <string.h>

#include "safe_queue.h"

/**
 * gets lock for reading an element
 * Call this function before reading an element
 * @param lock lock_custom that provides safety to the concerned element
 */
void get_read_lock(struct lock_custom *lock) {

    int count;

    sem_wait(lock->mut);        // lock(mutex)
    sem_post(lock->reader);    //read_count++

    sem_getvalue(lock->reader, &count);
    if (count == 1) {
        sem_wait(lock->writer);
    }

    sem_post(lock->mut);
}

/**
 * releases lock for reading an element
 * Call this function after reading an element
 * @param lock lock_custom that provides safety to the concerned element
 */
void realease_read_lock(struct lock_custom *lock) {

    int count;

    sem_wait(lock->mut);        // lock(mutex)
    sem_wait(lock->reader);    //read_count--

    sem_getvalue(lock->reader, &count);
    if (count == 0) {
        sem_post(lock->writer);
    }

    sem_post(lock->mut);
}

/**
 * gets lock for writing an element
 * Call this function before writing an element
 * @param lock lock_custom that provides safety to the concerned element
 */
void get_write_lock(struct lock_custom *lock) {
    sem_wait(lock->writer);
}

/**
 * release lock after writing an element
 * Call this function after writing an element
 * @param lock lock_custom that provides safety to the concerned element
 */
void realease_write_lock(struct lock_custom *lock) {
    sem_post(lock->writer);
}


/**
 * Prints the state of the locks
 * @param lock the lock_custom concerned
 */
void print_lock(struct lock_custom *lock) {

    int count_mut;
    sem_getvalue(lock->mut, &count_mut);
    printf("mut: %d\n", count_mut);

    int count_reader;
    sem_getvalue(lock->reader, &count_reader);
    printf("reader: %d\n", count_reader);

    int count_writer;
    sem_getvalue(lock->writer, &count_writer);
    printf("writer: %d\n", count_writer);

}


struct lock_custom *init_lock_custom(struct lock_custom *lock, char *id, int sync) {

    lock = (struct lock_custom *) malloc(sizeof(struct lock_custom *));

    char readers[100] = "readers";
    lock->reader = sem_open(strcat(readers, id), O_CREAT, 0600, 1);

    char writer[100] = "writer";
    lock->writer = sem_open(strcat(writer, id), O_CREAT, 0600, 1);

    char mut[100] = "mut";
    lock->mut = sem_open(strcat(mut, id), O_CREAT, 0600, 1);

    if (sync == 0) {
        // sem_post(lock->writer);
        // sem_post(lock->mut);
        sem_wait(lock->reader);
    }

    return lock;
}


struct safe_queue *init_queue(int id, struct safe_queue *queue, int sync) {

    int shm_id;
    key_t key = id;

    // Creating a segment
    if ((shm_id = shmget(key, sizeof(struct safe_queue), IPC_CREAT |  0600)) < 0) {
        perror("shm_get error");
        exit(1);
    }

    // shm is the link the shared mem
    if ((queue = (struct safe_queue *) shmat(shm_id, NULL, 0)) == (struct safe_queue *) -1) {
        perror("error in shmat");
        exit(1);
    }

    if (sync == CREAT) {
        // init
        queue->start_index = 0;
        queue->end_index = 0;

        // TODO: what about malloc?
    }

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
        sprintf(idd, "q%delement%d", id, i);
        queue->array[i].lock = *init_lock_custom(&queue->array[i].lock, idd, sync);
    }


    return queue;
}


void print_queue(struct safe_queue *queue, int lock) {

    printf("\n --------- Printing ------- \n");
    for (int i = queue->start_index,j=0; i < queue->end_index; ++i, ++j) {
        printf("---- Element #%d ---\n", j);
        printf("Data: %s\n", queue->array[i].data);
        if (lock == EVERYTHING) {
            print_lock(&queue->array[i].lock);
        }
    }
}

int queue_read_index(struct safe_queue *queue, int index) {

    // TODO: CHECK FOR RACE CONDITION  in-between the "||" dequeue or enqueue happens
    if (index < queue->start_index || index >= queue->end_index) {
        perror("Invalid index \n");
        return -1;
    }

    get_read_lock(&queue->array[index].lock);
    printf("Index: %d \t Content: %s\n", index, queue->array[index].data);
    realease_read_lock(&queue->array[index].lock);

    return 0;
}

int queue_write_index(struct safe_queue *queue, int index, char *str) {

    // TODO: CHECK FOR RACE CONDITION  in-between the "||" dequeue or enqueue happens
    if (index < queue->start_index || index >= queue->end_index) {
        perror("Invalid index \n");
        return -1;
    }

    get_write_lock(&queue->array[index].lock);
    strcpy(queue->array[index].data, str);
    realease_write_lock(&queue->array[index].lock);

    return 0;
}

int queue_enqueue(struct safe_queue *queue, char *str) {

    int err = 0;
    sem_wait(queue->enqueue_muttex);

    if (queue->start_index < MAX_CAPACITY - 1) {
        get_write_lock(&queue->array[queue->end_index].lock);
        strcpy(queue->array[queue->end_index++].data, str);
        realease_write_lock(&queue->array[queue->end_index - 1].lock);
    } else {
        err = -1;
        perror("Overflow");
    }

    sem_post(queue->enqueue_muttex);

    return err;

}

char *queue_dequeue(struct safe_queue *queue) {

    char *str = (char *) malloc(1024 * sizeof(char));

    sem_wait(queue->dequeue_muttex);

    if (queue->start_index < queue->end_index) {
        // TODO: RACE CONDITION.  queue->end_index might have been modified by now
        get_write_lock(&queue->array[queue->start_index].lock);

        strcpy(str, queue->array[queue->start_index].data);
        queue->start_index++;

        realease_write_lock(&queue->array[queue->start_index - 1].lock);
    } else {
        perror("Underflow");
        str = NULL;
    }

    sem_post(queue->dequeue_muttex);

    return str;


}

void destroy(struct safe_queue *queue, int id) {

    shmctl(shmget(id, sizeof(struct safe_queue), IPC_CREAT | 0666), IPC_RMID, NULL);

    sem_close(queue->dequeue_muttex);
    sem_close(queue->enqueue_muttex);

    sem_destroy(queue->enqueue_muttex);
    sem_destroy(queue->dequeue_muttex);

    for (int i = 0; i < MAX_CAPACITY; ++i) {
        sem_close(queue->array[i].lock.mut);
        sem_close(queue->array[i].lock.reader);
        sem_close(queue->array[i].lock.writer);

        sem_destroy(queue->array[i].lock.writer);
        sem_destroy(queue->array[i].lock.reader);
        sem_destroy(queue->array[i].lock.mut);
    }

}

int detach(struct safe_queue *queue) {
    return shmdt(queue);
}
