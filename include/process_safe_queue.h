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

#ifdef __cplusplus
extern "C" {
#endif

/*
 * The maximum length of the queue
 */
#define MAX_QUEUE_SIZE 20

/*
 * The max capacity of data that can be stored an element of the queue
 */
#define MAX_DATA_CAPACITY 1024

#define SYNC 1   // Used in init_queue
#define CREAT 0  // Used in init_queue

#define EVERYTHING 1  // Used with print_queue
#define ONLY_DATA 0   // Used with print queue

/**
 * Represents each element in the queue
 */
struct Element {
  void* buffer_ptr;    /* The data of the element buffer */
  uint32_t buffer_len; /* The len of the element buffer*/
};

struct Queue {
  int push_index;              /*general queue push index*/
  int pop_index;               /*general queue pop index*/
  int size;                    /*the size of queue element*/
  uint32_t buffer_size;        /*the capacity of one element buffer*/
  void* array[MAX_QUEUE_SIZE]; /*the array of buffer data structure to store all
                                  buffer contents in the queue*/
};

struct ProcessSafeQueue {
  key_t key_id;                 /* shm memory key id*/
  struct Queue* queue_data_ptr; /* shm memory data ptr*/
  sem_t* queue_mutex_ptr;       /*Ensures atomicity of enqueue operation */
};

/**
 * Before using ProcessSafeQueue you need init to get pointer to the queue data
 * structure using this function.
 *
 * @param id - Unique identifier for the queue
 * @param queue_ptr - the queue ptr
 * @param max_data_capacity - the capacity of each element buffer.
 *
 * ueue - queue to be used in future
 * @return a pointer the ProcessSafeQueue with @param id
 */
extern int Queue_Init(int id, struct ProcessSafeQueue* queue_ptr,
                      uint32_t max_data_capacity);

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
int Queue_Deinit(struct ProcessSafeQueue* queue_ptr);

#ifdef __cplusplus
}
#endif

#endif /*PROCESS_SAFE_QUEUE_H*/
