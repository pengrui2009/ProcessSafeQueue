/**
 * @file process_safe_queue.h
 * @author rui peng (pengrui_2009@163.com)
 * @brief 
 * @version 0.1
 * @date 2024-06-07
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef PROCESS_SAFE_QUEUE_H
#define PROCESS_SAFE_QUEUE_H

#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <string.h>

/*
 * The maximum length of the queue
 */
#define MAX_CAPACITY 10

/*
 * The max capacity of data that can be stored an element of the queue
 */
#define DATA_CAPACITY 1024


#define SYNC 1 // Used in init_queue
#define CREAT 0 // Used in init_queue

#define EVERYTHING 1 // Used with print_queue
#define ONLY_DATA 0 // Used with print queue


/**
 * lock_custom represents set of locks required for the safety of each queue element
 */
struct CustomLock {
    sem_t *reader;
    sem_t *writer;
    sem_t *mutex;
};

/**
 * Represents each element in the queue
 */
struct Element {
    uint8_t buffer_data[DATA_CAPACITY]; /* The data in the element*/
    uint32_t buffer_len;                /* The length in the element*/
    struct CustomLock lock; /* Sets of locks for the safety element*/
};

struct ProcessSafeQueue {
    int start_index;                    /*general queue start index*/
    int end_index;                      /*general queue end index*/
    int size;                           /*the size of queue element*/
    sem_t *enqueue_muttex;              /*Ensures atomicity of enqueue operation */
    sem_t *dequeue_muttex;              /*Ensures atomicity of dequeue operation */
    struct Element array[MAX_CAPACITY]; /*Inner data structure to store contents in the queue*/
};

/**
 * Before using ProcessSafeQueue you init get pointer to the queue data structure using this function.
 *
 * @param id - Unique identifier for the queue
 * @param q
 *
 * ueue - queue to be used in future
 * @param sync -
 * - CREAT:if using it for the first time.
 *      Creation will fail if a queue with the particular @param id has already been created
 * - SYNC: if the queue has already been created and you need a pointer to it
 * @return a pointer the ProcessSafeQueue with @param id
 */
int InitQueue(int id, struct ProcessSafeQueue **queue_ptr, int sync);

/**
 * Prints the content at index @param index of the ProcessSafeQueue
 * @param queue pointer to the safe queue
 * @param index starts from start_index to end_index.
 * index whose content you want to be printed
 * @returns 0 if successfull else -1
 */
int ReadQueueIndex(struct ProcessSafeQueue *queue_ptr, int index);

/**
 * Writes the content, @param str, at index, @param index. of the ProcessSafeQueue
 * @param queue pointer to the safe queue
 * @param index starts from start_index to end_index.index where you want to be writen
 * @param str the content you want to added
 * @returns 0 if successfull else -1
 */
int WriteQueueIndex(struct ProcessSafeQueue *queue_ptr, int index, char *str);

/**
 * Enqueues the content, str, in queue
 * @param queue
 * @param str The content that needs to be enqueued
 *  @returns 0 if successfull else -1 (Overflow)
 */

/**
 * @brief Enqueues the content, str, in queue
 * 
 * @param queue_ptr 
 * @param data_ptr :input data 
 * @param data_len :input data length
 * @return int : 0:success others:failed
 */
int Enqueue(struct ProcessSafeQueue *queue_ptr, 
            const void *data_ptr, const uint32_t data_len);

/**
 * @brief Dequeues an element from the queue
 * 
 * @param queue_ptr 
 * @param data_ptr  :output data 
 * @param data_size :output data size
 * @param data_len  :output data length
 * @return int : 0:success others:failed
 */
int Dequeue(struct ProcessSafeQueue *queue_ptr, void *data_ptr, 
              uint32_t data_size, uint32_t *data_len);

/**
 * Prints the queue in a weird format
 * NOT INTER PROCESS PROCESS SAFE
 * @param queue
 * @param lock
 *  - EVERYTHING : Prints the state of each lock along with content in each element of queue
 *  - ONLY_DATA: Prints the content of the queue
 */
void PrintQueue(struct ProcessSafeQueue *queue_ptr, int lock);


/**
 * Tries to destroy the queue.
 * NOT SAFE TO USE
 * @param queue a link to the queue
 * @param id the id of the queue
 */
void Destroy(struct ProcessSafeQueue *queue_ptr, int id);

/**
 * Detaches the current pointer of queue to the shared queue.
 * Coo?
 * @param queue
 * @return 0 if successful else * ERROR
 */
int Detach(struct ProcessSafeQueue *queue_ptr);

#endif /*PROCESS_SAFE_QUEUE_H*/