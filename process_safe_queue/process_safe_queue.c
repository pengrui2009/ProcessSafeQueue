/**
 * @file process_safe_queue.c
 * @author rui.peng (pengrui_2009@163.com)
 * @brief 
 * @version 0.1
 * @date 2024-06-07
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "process_safe_queue.h"
#include <stdio.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/**
 * gets lock for reading an element
 * Call this function before reading an element
 * @param lock lock_custom that provides safety to the concerned element
 */
static void GetReadLock(struct CustomLock *lock_ptr) {

    int count;

    sem_wait(lock_ptr->mut);        // lock(mutex)
    sem_post(lock_ptr->reader);    //read_count++

    sem_getvalue(lock_ptr->reader, &count);
    if (count == 1) {
        sem_wait(lock_ptr->writer);
    }

    sem_post(lock_ptr->mut);
}

/**
 * releases lock for reading an element
 * Call this function after reading an element
 * @param lock lock_custom that provides safety to the concerned element
 */
static void RealeaseReadLock(struct CustomLock *lock_ptr) {

    int count;

    sem_wait(lock_ptr->mut);        // lock(mutex)
    sem_wait(lock_ptr->reader);    //read_count--

    sem_getvalue(lock_ptr->reader, &count);
    if (count == 0) {
        sem_post(lock_ptr->writer);
    }

    sem_post(lock_ptr->mut);
}

/**
 * gets lock for writing an element
 * Call this function before writing an element
 * @param lock lock_custom that provides safety to the concerned element
 */
static void GetWriteLock(struct CustomLock *lock_ptr) {
    sem_wait(lock_ptr->writer);
}

/**
 * release lock after writing an element
 * Call this function after writing an element
 * @param lock lock_custom that provides safety to the concerned element
 */
static void RealeaseWriteLock(struct CustomLock *lock_ptr) {
    sem_post(lock_ptr->writer);
}


/**
 * Prints the state of the locks
 * @param lock the lock_custom concerned
 */
void print_lock(struct CustomLock *lock_ptr) {

    int count_mut;
    sem_getvalue(lock_ptr->mut, &count_mut);
    printf("mut: %d\n", count_mut);

    int count_reader;
    sem_getvalue(lock_ptr->reader, &count_reader);
    printf("reader: %d\n", count_reader);

    int count_writer;
    sem_getvalue(lock_ptr->writer, &count_writer);
    printf("writer: %d\n", count_writer);

}


static int InitCustomLock(struct CustomLock *lock_ptr, char *id_ptr, int sync) {

    int result = -1;
    // lock_ptr = (struct CustomLock *) malloc(sizeof(struct CustomLock));

    char readers[100] = "readers";
    lock_ptr->reader = sem_open(strcat(readers, id_ptr), O_CREAT, 0600, 1);
    if (SEM_FAILED == lock_ptr->reader)
    {
        result = -1;
        return result;
    }
    char writer[100] = "writer";
    lock_ptr->writer = sem_open(strcat(writer, id_ptr), O_CREAT, 0600, 1);
    if (SEM_FAILED == lock_ptr->writer)
    {
        result = -1;
        return result;
    }
    char mut[100] = "mut";
    lock_ptr->mut = sem_open(strcat(mut, id_ptr), O_CREAT, 0600, 1);
    if (SEM_FAILED == lock_ptr->mut)
    {
        result = -1;
        return result;
    }

    if (sync == 0) {
        // sem_post(lock->writer);
        // sem_post(lock->mut);
        sem_wait(lock_ptr->reader);
    }

    // return lock_ptr;
    result = 0;
    return result;
}

/**
 * @brief initialize process safe queue
 * 
 * @param id 
 * @param queue_ptr 
 * @param sync 
 * @return struct ProcessSafeQueue* 
 */
int InitQueue(int id, struct ProcessSafeQueue **queue_ptr, int sync) {

    int result = -1;
    int shm_id;
    key_t key = id;

    // Creating a segment
    if ((shm_id = shmget(key, sizeof(struct ProcessSafeQueue), IPC_CREAT |  0600)) < 0) {
        perror("shm_get error");
        result = -1;
        return result;
    }

    // shm is the link the shared mem
    if ((*queue_ptr = (struct ProcessSafeQueue *)shmat(shm_id, NULL, 0)) == (struct ProcessSafeQueue *) -1) {
        perror("error in shmat");
        shmctl(shmget(shm_id, sizeof(struct ProcessSafeQueue), IPC_CREAT | 0666), IPC_RMID, NULL);
        result = -2;
        return result;
    }

    if (sync == CREAT) {
        // init
        (*queue_ptr)->start_index = 0;
        (*queue_ptr)->end_index = 0;
        (*queue_ptr)->size = 0;
        // TODO: what about malloc?
    }

    /*
        Setting up queue mutex
    */
    char enqueue_muttex_str[100];
    sprintf(enqueue_muttex_str, "enqueue_muttex%d", id);
    (*queue_ptr)->enqueue_muttex = sem_open(enqueue_muttex_str, O_CREAT, 0600, 1);
    // sem_post(queue->enqueue_muttex);

    char dequeue_muttex_str[100];
    sprintf(dequeue_muttex_str, "dequeue_muttex%d", id);
    (*queue_ptr)->dequeue_muttex = sem_open(dequeue_muttex_str, O_CREAT, 0600, 1);
    // sem_post(queue->dequeue_muttex);
    result = 0;
    
    for (int i = 0; i < MAX_CAPACITY; ++i) {
        char idd[50];
        sprintf(idd, "q%delement%d", id, i);
        result = InitCustomLock(&(*queue_ptr)->array[i].lock, idd, sync);
        if (result)
        {
            break;
        }
    }

    return result;
}


void PrintQueue(struct ProcessSafeQueue *queue_ptr, int lock) {

    printf("\n --------- Printing ------- \n");
    for (int i = queue_ptr->start_index,j=0; i < queue_ptr->end_index; ++i, ++j) {
        printf("---- Element #%d ---\n", j);
        printf("Data: %s\n", queue_ptr->array[i].buffer_data);
        if (lock == EVERYTHING) {
            print_lock(&queue_ptr->array[i].lock);
        }
    }
}

int ReadQueueIndex(struct ProcessSafeQueue *queue_ptr, int index) {

    // TODO: CHECK FOR RACE CONDITION  in-between the "||" dequeue or enqueue happens
    if (index < queue_ptr->start_index || index >= queue_ptr->end_index) {
        perror("Invalid index \n");
        return -1;
    }

    GetReadLock(&queue_ptr->array[index].lock);
    printf("Index: %d \t Content: %s\n", index, (char *)queue_ptr->array[index].buffer_data);
    RealeaseReadLock(&queue_ptr->array[index].lock);

    return 0;
}

int WriteQueueIndex(struct ProcessSafeQueue *queue_ptr, int index, char *str) {

    // TODO: CHECK FOR RACE CONDITION  in-between the "||" dequeue or enqueue happens
    if (index < queue_ptr->start_index || index >= queue_ptr->end_index) {
        perror("Invalid index \n");
        return -1;
    }

    GetWriteLock(&queue_ptr->array[index].lock);
    strcpy((char *)queue_ptr->array[index].buffer_data, str);
    RealeaseWriteLock(&queue_ptr->array[index].lock);

    return 0;
}

int Enqueue(struct ProcessSafeQueue *queue_ptr, const void *data_ptr, 
            const uint32_t data_len) {

    int result = -1;

    if ((NULL == queue_ptr) || (NULL == data_ptr))
    {
        result = -1;
        return result;
    }

    if (DATA_CAPACITY < data_len)
    {
        result = -2;
        return result;
    }

    sem_wait(queue_ptr->enqueue_muttex);
    if (queue_ptr->size < DATA_CAPACITY)
    {
        GetWriteLock(&queue_ptr->array[queue_ptr->end_index].lock);
        memcpy(queue_ptr->array[queue_ptr->end_index].buffer_data, data_ptr, data_len);
        queue_ptr->array[queue_ptr->end_index].buffer_len = data_len;
        RealeaseWriteLock(&queue_ptr->array[queue_ptr->end_index].lock);
        queue_ptr->end_index++;
        if ((MAX_CAPACITY-1) < queue_ptr->end_index)
        {
            queue_ptr->end_index = 0;
        }
        queue_ptr->size++;
        result = 0;
    } else {
        result = -1;
        perror("Overflow");
    }

    sem_post(queue_ptr->enqueue_muttex);

    return result;

}

/**
 * @brief dequeue element from process safe queue
 * 
 * @param queue_ptr 
 * @param data_ptr 
 * @param data_size 
 * @param data_len 
 * @return int 
 */
int Dequeue(struct ProcessSafeQueue *queue_ptr, void *data_ptr, 
            uint32_t data_size, uint32_t *data_len) {
    
    int result = -1;

    if ((NULL == queue_ptr) || (NULL == data_ptr) || (NULL == data_len))
    {
        result = -1;
        return result;
    }

    if (DATA_CAPACITY < data_size)
    {
        result = -2;
        return result;
    }

    sem_wait(queue_ptr->dequeue_muttex);

    if (queue_ptr->size > 0)
    {
        GetWriteLock(&queue_ptr->array[queue_ptr->start_index].lock);

        memcpy(data_ptr, queue_ptr->array[queue_ptr->start_index].buffer_data, 
               queue_ptr->array[queue_ptr->start_index].buffer_len);
        *data_len = queue_ptr->array[queue_ptr->start_index].buffer_len;
        queue_ptr->start_index++;
        RealeaseWriteLock(&queue_ptr->array[queue_ptr->start_index-1].lock);
        if ((MAX_CAPACITY-1) < queue_ptr->start_index)
        {
            queue_ptr->start_index = 0;
        } 
        queue_ptr->size--;
        result = 0;
    } else {
        result = -1;
        perror("Underflow");
    }

    sem_post(queue_ptr->dequeue_muttex);

    return result;
}

/**
 * @brief destory process safe queue
 * 
 * @param queue_ptr 
 * @param id 
 */
void Destroy(struct ProcessSafeQueue *queue_ptr, int id) {

    shmctl(shmget(id, sizeof(struct ProcessSafeQueue), IPC_CREAT | 0666), IPC_RMID, NULL);

    sem_close(queue_ptr->dequeue_muttex);
    sem_close(queue_ptr->enqueue_muttex);

    sem_destroy(queue_ptr->enqueue_muttex);
    sem_destroy(queue_ptr->dequeue_muttex);

    for (int i = 0; i < MAX_CAPACITY; ++i) {
        sem_close(queue_ptr->array[i].lock.mut);
        sem_close(queue_ptr->array[i].lock.reader);
        sem_close(queue_ptr->array[i].lock.writer);

        sem_destroy(queue_ptr->array[i].lock.writer);
        sem_destroy(queue_ptr->array[i].lock.reader);
        sem_destroy(queue_ptr->array[i].lock.mut);
    }

}

/**
 * @brief detach process safe queue
 * 
 * @param queue_ptr 
 * @return int 
 */
int Detach(struct ProcessSafeQueue *queue_ptr) {
    return shmdt(queue_ptr);
}