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

/**
 * @brief initialize process safe queue
 *
 * @param id                - Unique identifier for the queue
 * @param queue_ptr         - Initialize queue point(output)
 * @param sync              - Initialize method:CREAT or SYNC
 * @return struct ProcessSafeQueue*
 */
int Queue_Init(int id, struct ProcessSafeQueue* queue_ptr, int sync) {
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
    perror("error in shmat");
    shmctl(shm_id, IPC_RMID, NULL);
    result = -ERR_SYS_SHMAT;
    return result;
  }

  if (sync == CREAT) {
    // init
    queue_ptr->queue_data_ptr->pop_index = 0;
    queue_ptr->queue_data_ptr->push_index = 0;
    queue_ptr->queue_data_ptr->size = 0;
    // TODO: what about malloc?
  }

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
                    ? (queue_ptr->queue_data_ptr->push_index + MAX_CAPACITY)
                    : queue_ptr->queue_data_ptr->push_index;
    for (int i = pop_index, j = 0; i < push_index; ++i, ++j) {
      printf("---- Element #%d ---\n", j);
      printf("Len:%d Data: %s\n",
             queue_ptr->queue_data_ptr->array[i % MAX_CAPACITY].buffer_len,
             (char*)(queue_ptr->queue_data_ptr->array[i % MAX_CAPACITY]
                         .buffer_data));
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

  if (DATA_CAPACITY < data_len) {
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

  if (queue_ptr->queue_data_ptr->size < MAX_CAPACITY) {
    memcpy(
        queue_ptr->queue_data_ptr->array[queue_ptr->queue_data_ptr->push_index]
            .buffer_data,
        data_ptr, data_len);
    queue_ptr->queue_data_ptr->array[queue_ptr->queue_data_ptr->push_index]
        .buffer_len = data_len;
    queue_ptr->queue_data_ptr->push_index++;
    if ((MAX_CAPACITY - 1) < queue_ptr->queue_data_ptr->push_index) {
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

  if (DATA_CAPACITY < data_size) {
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
            .buffer_data,
        queue_ptr->queue_data_ptr->array[queue_ptr->queue_data_ptr->pop_index]
            .buffer_len);
    *data_len =
        queue_ptr->queue_data_ptr->array[queue_ptr->queue_data_ptr->pop_index]
            .buffer_len;
    queue_ptr->queue_data_ptr->pop_index++;

    if ((MAX_CAPACITY - 1) < queue_ptr->queue_data_ptr->pop_index) {
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

  if (DATA_CAPACITY < data_size) {
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
            .buffer_data,
        queue_ptr->queue_data_ptr->array[queue_ptr->queue_data_ptr->pop_index]
            .buffer_len);
    *data_len =
        queue_ptr->queue_data_ptr->array[queue_ptr->queue_data_ptr->pop_index]
            .buffer_len;
    queue_ptr->queue_data_ptr->pop_index++;

    if ((MAX_CAPACITY - 1) < queue_ptr->queue_data_ptr->pop_index) {
      queue_ptr->queue_data_ptr->pop_index = 0;
    }
    queue_ptr->queue_data_ptr->size--;
    result = 0;
  } else {
    result = -ERR_QUEUE_UNDERFLOW;
    perror("Underflow");
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
 * @brief destory process safe queue
 *
 * @param queue_ptr
 * @param id
 */
static int Queue_Destroy(struct ProcessSafeQueue* queue_ptr, int id) {
  int result = -1;
  int shmid = -1;

  result = shmget(id, sizeof(struct ProcessSafeQueue), IPC_CREAT | 0666);
  if (-1 == result)
  {
    result = -ERR_SYS_SHMGET;
    return result;
  }
  shmid = result;
  result = shmctl(shmid, IPC_RMID, NULL);
  if (-1 == result)
  {
    result = -ERR_SYS_SHMCTRL;
    return result;
  }
  result = sem_close(queue_ptr->queue_mutex_ptr);
  if (-1 == result)
  {
    result = -ERR_SYS_SEMCLOSE;
    return result;
  }
  result = sem_destroy(queue_ptr->queue_mutex_ptr);
  if (-1 == result)
  {
    result = -ERR_SYS_SEMDESTROY;
    return result;
  }

  return result;
}

/**
 * @brief detach process safe queue
 *
 * @param queue_ptr
 * @return int
 */
static int Queue_Detach(struct ProcessSafeQueue* queue_ptr) {
  int result = -1;
  result = shmdt(queue_ptr->queue_data_ptr);
  if (result)
  {
    result = -ERR_SYS_SHMDT;
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

  result = Queue_Detach(queue_ptr);
  if (result)
  {
    return result;
  }

  result = Queue_Destroy(queue_ptr, id);
  if (result)
  {
    return result;
  }

  return result;
}

#ifdef __cplusplus
}
#endif
