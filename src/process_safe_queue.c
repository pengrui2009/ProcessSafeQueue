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

#include <fcntl.h>
#include <semaphore.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>

#include "error.h"

#ifdef __cplusplus
extern "C"{
#endif

static int Shm_Init(key_t key, void **shm_ptr)
{
  int result = 0;

  return result;
}
/**
 * @brief initialize process safe queue
 *
 * @param id                - Unique identifier for the queue
 * @param queue_ptr         - Initialize queue point(output)
 * @param max_data_capacity - the capacity of each queue data buffer
 * @return struct ProcessSafeQueue*
 */
int Queue_Init(int id, struct ProcessSafeQueue* queue_ptr, uint32_t max_data_capacity) {
  int result = -1;
  int shm_id = -1;
  key_t key = id;

  if (NULL == queue_ptr) {
    result = -ERR_INVAL_NULLPOINT;
    return result;
  }

  // Creating a segment
  result = shmget(key, sizeof(struct Queue), IPC_CREAT | 0600);
  if (-1 == result) {
    result = -ERR_SYS_SHMGET;
    return result;
  }
  shm_id = result;
  // shm is the link the shared mem
  if ((queue_ptr->queue_data_ptr = (struct Queue*)shmat(shm_id, NULL, 0)) ==
      (struct Queue*)-1) {
    shmctl(shm_id, IPC_RMID, NULL);
    result = -ERR_SYS_SHMAT;
    return result;
  }
  queue_ptr->queue_data_ptr->shm_id = shm_id;

  // if (sync == CREAT) {
  //   // init
  //   queue_ptr->queue_data_ptr->pop_index = 0;
  //   queue_ptr->queue_data_ptr->push_index = 0;
  //   queue_ptr->queue_data_ptr->size = 0;
  //   queue_ptr->queue_data_ptr->buffer_size = max_data_capacity;
  //   // TODO: what about malloc?
  // }
  // when shm not exist, then update max_data_capacity
  queue_ptr->queue_data_ptr->buffer_size = (queue_ptr->queue_data_ptr->buffer_size > 0) ? 
      queue_ptr->queue_data_ptr->buffer_size : max_data_capacity;

  /*
      Setting up queue mutex
  */
  // char enqueue_muttex_str[100];
  // sprintf(enqueue_muttex_str, "enqueue_muttex%d", id);
  // (*queue_ptr)->enqueue_muttex = sem_open(enqueue_muttex_str, O_CREAT, 0600,
  // 1); sem_post(queue->enqueue_muttex);

  char queue_mutex_str[100];
  sprintf(queue_mutex_str, "queue_muttex%d", id);
  queue_ptr->queue_mutex_ptr = sem_open(queue_mutex_str, O_CREAT, 0600, 1);
  if (SEM_FAILED == queue_ptr->queue_mutex_ptr) {
    // detach the shared memory
    shmdt(queue_ptr->queue_data_ptr);
    // destory the shared memory
    shmctl(shm_id, IPC_RMID, NULL);

    result = -ERR_SYS_SEMOPEN;
    return result;
  }

  // attached each buffer of queue to shared memory
  for (int index =0 ; index < MAX_QUEUE_SIZE; index++) {
    // Creating a segment
    int data_shm_id = -1;
    key_t data_key = key + index + 1;
    result = shmget(data_key, queue_ptr->queue_data_ptr->buffer_size, IPC_CREAT | 0600);
    if (-1 == result) {
      result = -ERR_SYS_SHMGET;
      return result;
    }
    data_shm_id = result;
    // shm is the link the shared mem
    if ((queue_ptr->queue_data_ptr->array[index].buffer_ptr = (void *)shmat(data_shm_id, NULL, 0)) ==
        (void *)-1) {
      shmctl(data_shm_id, IPC_RMID, NULL);
      result = -ERR_SYS_SHMAT;
      return result;
    }
    queue_ptr->queue_data_ptr->array[index].shm_id = data_shm_id;
  }
  // sem_post(queue->dequeue_muttex);
  result = 0;

  return result;
}

/**
 * @brief print all elements in process safe queue
 *
 * @param queue_ptr
 */
void Queue_Print(struct ProcessSafeQueue* queue_ptr) {
  if (NULL == queue_ptr)
  {
      return;
  }

  printf("\n --------- Printing ------- \n");
  printf("pop_index:%d push_index:%d size:%d\n",
         queue_ptr->queue_data_ptr->pop_index,
         queue_ptr->queue_data_ptr->push_index, queue_ptr->queue_data_ptr->size);
  int pop_index = queue_ptr->queue_data_ptr->pop_index;
  int push_index = 0;
  if (queue_ptr->queue_data_ptr->size) {
    push_index = (queue_ptr->queue_data_ptr->pop_index >=
                 queue_ptr->queue_data_ptr->push_index)
                    ? (queue_ptr->queue_data_ptr->push_index + MAX_QUEUE_SIZE)
                    : queue_ptr->queue_data_ptr->push_index;
    for (int i = pop_index, j = 0; i < push_index; ++i, ++j) {
      printf("---- Element #%d ---\n", j);
      printf("Len:%d Data: %s\n",
             queue_ptr->queue_data_ptr->array[i % MAX_QUEUE_SIZE].buffer_len,
             (char*)(queue_ptr->queue_data_ptr->array[i % MAX_QUEUE_SIZE]
                         .buffer_ptr));
    }
  }

  printf(" -------------------------- \n");
}

/**
 * @brief push element into process safe queue
 *
 * @param queue_ptr
 * @param data_ptr
 * @param data_len
 * @return int
 */
int Queue_Push(struct ProcessSafeQueue* queue_ptr, const void* data_ptr,
               const uint32_t data_len) {
  int result = -1;

  if ((NULL == queue_ptr) || (NULL == data_ptr)) {
    result = -ERR_INVAL_NULLPOINT;
    return result;
  }

  if (data_len > queue_ptr->queue_data_ptr->buffer_size) {
    result = -ERR_INVAL_MEMSIZE;
    return result;
  }
  // int value = -1;
  // sem_post(queue_mutex_ptr);
  // sem_getvalue(queue_ptr->queue_mutex_ptr, &value);
  // printf("EnQueue sem value:%d\n", value);
  result = sem_wait(queue_ptr->queue_mutex_ptr);
  if (-1 == result) {
    result = -ERR_SYS_SEMWAIT;
    return result;
  }

  if (queue_ptr->queue_data_ptr->size < MAX_QUEUE_SIZE) {
    memcpy(
        queue_ptr->queue_data_ptr->array[queue_ptr->queue_data_ptr->push_index]
            .buffer_ptr,

        data_ptr, data_len);
    queue_ptr->queue_data_ptr->array[queue_ptr->queue_data_ptr->push_index]
        .buffer_len = data_len;
    queue_ptr->queue_data_ptr->push_index++;
    if ((MAX_QUEUE_SIZE - 1) < queue_ptr->queue_data_ptr->push_index) {
      queue_ptr->queue_data_ptr->push_index = 0;
    }
    queue_ptr->queue_data_ptr->size++;
    result = 0;
  } else {
    result = -ERR_QUEUE_OVERFLOW;
  }

  if (-1 == sem_post(queue_ptr->queue_mutex_ptr)) {
    result = -ERR_SYS_SEMPOST;
    return result;
  }

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
int Queue_Pop(struct ProcessSafeQueue* queue_ptr, void* data_ptr,
              uint32_t data_size, uint32_t* data_len) {
  int result = -1;

  if ((NULL == queue_ptr) || (NULL == data_ptr) || (NULL == data_len)) {
    result = -ERR_INVAL_NULLPOINT;
    return result;
  }

  if (data_size < queue_ptr->queue_data_ptr->buffer_size) {
    result = -ERR_INVAL_MEMSIZE;
    return result;
  }

  // int value = -1;
  // sem_wait(queue_mutex_ptr);
  // sem_getvalue(queue_ptr->queue_mutex_ptr, &value);
  // printf("DeQueue sem value:%d\n", value);
  result = sem_wait(queue_ptr->queue_mutex_ptr);
  if (-1 == result) {
    result = -ERR_SYS_SEMWAIT;
    return result;
  }

  if (queue_ptr->queue_data_ptr->size > 0) {
    memcpy(
        data_ptr,
        queue_ptr->queue_data_ptr->array[queue_ptr->queue_data_ptr->pop_index]
            .buffer_ptr,
        queue_ptr->queue_data_ptr->array[queue_ptr->queue_data_ptr->pop_index]
            .buffer_len);
    *data_len =
        queue_ptr->queue_data_ptr->array[queue_ptr->queue_data_ptr->pop_index]
            .buffer_len;
    queue_ptr->queue_data_ptr->pop_index++;

    if ((MAX_QUEUE_SIZE - 1) < queue_ptr->queue_data_ptr->pop_index) {
      queue_ptr->queue_data_ptr->pop_index = 0;
    }
    queue_ptr->queue_data_ptr->size--;
    result = 0;
  } else {
    result = -ERR_QUEUE_UNDERFLOW;
  }

  if (-1 == sem_post(queue_ptr->queue_mutex_ptr)) {
    result = -ERR_SYS_SEMPOST;
    return result;
  }

  return result;
}

/**
 * @brief wait and pop queue with timeout
 * 
 * @param queue_ptr : thread safe queue ptr[input]
 * @param data_ptr  : data buffer ptr[output]
 * @param data_size : the size of data buffer [input]
 * @param data_len  : the get length of data buffer [output]
 * @param timeout   : timeout (unit:1ms)
 * @return int      : 0: success other: failed
 */
int Queue_Wait_Pop(struct ProcessSafeQueue* queue_ptr, void* data_ptr,
                   uint32_t data_size, uint32_t* data_len, uint32_t timeout) {
  int result = -1;
  struct timespec to;

  if ((NULL == queue_ptr) || (NULL == data_ptr) || (NULL == data_len)) {
    result = -ERR_INVAL_NULLPOINT;
    return result;
  }

  if (data_size < queue_ptr->queue_data_ptr->buffer_size) {
    result = -ERR_INVAL_MEMSIZE;
    return result;
  }

  // int value = -1;
  // sem_wait(queue_mutex_ptr);
  // sem_getvalue(queue_ptr->queue_mutex_ptr, &value);
  // printf("DeQueue sem value:%d\n", value);

  to.tv_sec = time(NULL) + timeout / 1000;
  to.tv_nsec = (long)(timeout % 1000) * 1000000000L;
  if (sem_timedwait(queue_ptr->queue_mutex_ptr, &to)) {
    result = -ERR_SYS_SEMTIMEWAIT;
    return result;
  }

  if (queue_ptr->queue_data_ptr->size > 0) {
    memcpy(
        data_ptr,
        queue_ptr->queue_data_ptr->array[queue_ptr->queue_data_ptr->pop_index]
            .buffer_ptr,
        queue_ptr->queue_data_ptr->array[queue_ptr->queue_data_ptr->pop_index]
            .buffer_len);
    *data_len =
        queue_ptr->queue_data_ptr->array[queue_ptr->queue_data_ptr->pop_index]
            .buffer_len;
    queue_ptr->queue_data_ptr->pop_index++;

    if ((MAX_QUEUE_SIZE - 1) < queue_ptr->queue_data_ptr->pop_index) {
      queue_ptr->queue_data_ptr->pop_index = 0;
    }
    queue_ptr->queue_data_ptr->size--;
    result = 0;
  } else {
    result = -ERR_QUEUE_UNDERFLOW;
  }

  if (-1 == sem_post(queue_ptr->queue_mutex_ptr)) {
    result = -ERR_SYS_SEMPOST;
    return result;
  }
  return result;
}

/**
 * @brief get the queue size
 * 
 * @param queue_ptr : queue ptr
 * @return int      : the size of queue element 
 */
int Queue_Size(struct ProcessSafeQueue* queue_ptr) {
  int result = 0;

  result = sem_wait(queue_ptr->queue_mutex_ptr);
  if (-1 == result) {
    result = -ERR_SYS_SEMWAIT;
    return result;
  }

  result = queue_ptr->queue_data_ptr->size;

  if (-1 == sem_post(queue_ptr->queue_mutex_ptr)) {
    result = -ERR_SYS_SEMPOST;
    return result;
  }
  return result;
}

/**
 * @brief is queue empty
 * 
 * @param queue_ptr : queue ptr
 * @return int      : 1:empty 0:not empty
 */
int Queue_IsEmpty(struct ProcessSafeQueue* queue_ptr) {
  int result = -1;
  result = sem_wait(queue_ptr->queue_mutex_ptr);
  if (-1 == result) {
    result = -ERR_SYS_SEMWAIT;
    return result;
  }

  result = (queue_ptr->queue_data_ptr->size == 0) ? 1 : 0;
  
  if (-1 == sem_post(queue_ptr->queue_mutex_ptr)) {
    result = -ERR_SYS_SEMPOST;
    return result;
  }
  return result;
}

/**
 * @brief Deinitialize process safe queue
 *
 * @param queue_ptr
 * @return int
 */
int Queue_Deinit(struct ProcessSafeQueue* queue_ptr, int id) {
  int result = -1;
  int queue_shmid = 0;
  int element_shmid[MAX_QUEUE_SIZE] = {0};

  // detach shared memory
  for (int index = 0; index < MAX_QUEUE_SIZE ; index++)
  {
    element_shmid[index] = queue_ptr->queue_data_ptr->array[index].shm_id;
    result = shmdt(queue_ptr->queue_data_ptr->array[index].buffer_ptr);
    if (result)
    {
      result = -ERR_SYS_SHMDT;
      return result;
    }
  }

  queue_shmid = queue_ptr->queue_data_ptr->shm_id;
  result = shmdt(queue_ptr->queue_data_ptr);
  if (result)
  {
    result = -ERR_SYS_SHMDT;
    return result;
  }

  // delete shared memroy
  for (int index = 0; index < MAX_QUEUE_SIZE; index++)
  {
    result = shmctl(element_shmid[index], IPC_RMID, NULL);
    if (-1 == result)
    {
      result = -ERR_SYS_SHMCTRL;
      return result;
    }
  }

  result = shmctl(queue_shmid, IPC_RMID, NULL);
  if (-1 == result)
  {
    result = -ERR_SYS_SHMCTRL;
    return result;
  }

  // sem destory
  result = sem_close(queue_ptr->queue_mutex_ptr);
  if (-1 == result)
  {
    result = -ERR_SYS_SEMCLOSE;
    return result;
  }

  return result;
}

#ifdef __cplusplus
}
#endif
