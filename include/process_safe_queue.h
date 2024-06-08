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

#include <fcntl.h>
#include <semaphore.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>

/*
 * The maximum length of the queue
 */
#define MAX_CAPACITY 10

/*
 * The max capacity of data that can be stored an element of the queue
 */
#define DATA_CAPACITY 1024

#define SYNC 1   // Used in init_queue
#define CREAT 0  // Used in init_queue

#define EVERYTHING 1  // Used with print_queue
#define ONLY_DATA 0   // Used with print queue

/**
 * Represents each element in the queue
 */
struct Element {
  uint8_t buffer_data[DATA_CAPACITY]; /* The data in the element*/
  uint32_t buffer_len;                /* The length in the element*/
};

struct Queue {
  int start_index;                    /*general queue start index*/
  int end_index;                      /*general queue end index*/
  int size;                           /*the size of queue element*/
  struct Element array[MAX_CAPACITY]; /*Inner data structure to store contents
                                         in the queue*/
};

struct ProcessSafeQueue {
  struct Queue* queue_data_ptr;
  sem_t* queue_mutex_ptr; /*Ensures atomicity of enqueue operation */
};

/**
 * Before using ProcessSafeQueue you need init to get pointer to the queue data
 * structure using this function.
 *
 * @param id - Unique identifier for the queue
 * @param q
 *
 * ueue - queue to be used in future
 * @param sync -
 * - CREAT:if using it for the first time.
 *      Creation will fail if a queue with the particular @param id has already
 * been created
 * - SYNC: if the queue has already been created and you need a pointer to it
 * @return a pointer the ProcessSafeQueue with @param id
 */
int Queue_Init(int id, struct ProcessSafeQueue* queue_ptr, int sync);

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
int Queue_Push(struct ProcessSafeQueue* queue_ptr, const void* data_ptr,
               const uint32_t data_len);

/**
 * @brief Dequeues an element from the queue
 *
 * @param queue_ptr
 * @param data_ptr  :output data
 * @param data_size :output data size
 * @param data_len  :output data length
 * @return int : 0:success others:failed
 */
int Queue_Pop(struct ProcessSafeQueue* queue_ptr, void* data_ptr,
              uint32_t data_size, uint32_t* data_len);

/**
 * @brief wait and dequeue from process safe queue
 *
 * @param queue_ptr
 * @param data_ptr
 * @param data_size
 * @param data_len
 * @param timeout
 * @return int
 */
int Queue_Wait_Pop(struct ProcessSafeQueue* queue_ptr, void* data_ptr,
                   uint32_t data_size, uint32_t* data_len, uint32_t timeout);
/**
 * Prints the queue in a weird format
 * NOT INTER PROCESS PROCESS SAFE
 * @param queue
 *  - EVERYTHING : Prints the state of each lock along with content in each
 * element of queue
 *  - ONLY_DATA: Prints the content of the queue
 */
void Queue_Print(struct ProcessSafeQueue* queue_ptr);

/**
 * @brief Get the Queue Size object
 *
 * @param queue_ptr
 * @return int  : size of queue
 */
int Queue_Size(struct ProcessSafeQueue* queue_ptr);

/**
 * @brief is queue empty
 *
 * @param queue_ptr
 * @return int  : 0:not empty, 1:empty
 */
int Queue_IsEmpty(struct ProcessSafeQueue* queue_ptr);

/**
 * @brief Deinitialize process safe queue
 *
 * @param queue_ptr
 * @return int
 */
int Queue_Deinit(struct ProcessSafeQueue* queue_ptr, int id);

#endif /*PROCESS_SAFE_QUEUE_H*/