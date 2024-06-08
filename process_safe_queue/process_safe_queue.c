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
#include <time.h>
/**
 * @brief initialize process safe queue
 * 
 * @param id 
 * @param psafeqeue_ptr 
 * @param sync 
 * @return struct ProcessSafeQueue* 
 */
int InitQueue(int id, struct ProcessSafeQueue *queue_ptr, int sync) {

    int result = -1;
    int shm_id;
    key_t key = id;

    // Creating a segment
    if ((shm_id = shmget(key, sizeof(struct Queue), IPC_CREAT |  0600)) < 0) {
        perror("shm_get error");
        result = -1;
        return result;
    }

    // shm is the link the shared mem
    if ((queue_ptr->queue_data_ptr = (struct Queue *)shmat(shm_id, NULL, 0)) == (struct Queue *) -1) {
        perror("error in shmat");
        shmctl(shmget(shm_id, sizeof(struct Queue), IPC_CREAT | 0666), IPC_RMID, NULL);
        result = -2;
        return result;
    }

    if (sync == CREAT) {
        // init
        queue_ptr->queue_data_ptr->start_index = 0;
        queue_ptr->queue_data_ptr->end_index = 0;
        queue_ptr->queue_data_ptr->size = 0;
        // TODO: what about malloc?
    }

    /*
        Setting up queue mutex
    */
    // char enqueue_muttex_str[100];
    // sprintf(enqueue_muttex_str, "enqueue_muttex%d", id);
    // (*queue_ptr)->enqueue_muttex = sem_open(enqueue_muttex_str, O_CREAT, 0600, 1);
    // sem_post(queue->enqueue_muttex);

    char queue_mutex_str[100];
    sprintf(queue_mutex_str, "queue_muttex%d", id);
    queue_ptr->queue_mutex_ptr = sem_open(queue_mutex_str, O_CREAT, 0600, 1);
    // sem_post(queue->dequeue_muttex);
    result = 0;

    return result;
}


void PrintQueue(struct ProcessSafeQueue *queue_ptr, int lock) {

    printf("\n --------- Printing ------- \n");
    printf("start_index:%d end_index:%d size:%d\n", queue_ptr->queue_data_ptr->start_index, 
        queue_ptr->queue_data_ptr->end_index, queue_ptr->queue_data_ptr->size);
    int start_index = queue_ptr->queue_data_ptr->start_index;
    int end_index = 0;
    if (queue_ptr->queue_data_ptr->size)
    {
        end_index = (queue_ptr->queue_data_ptr->start_index >= queue_ptr->queue_data_ptr->end_index) ? 
                   (queue_ptr->queue_data_ptr->end_index + MAX_CAPACITY) : queue_ptr->queue_data_ptr->end_index;
    } else {
        end_index = (queue_ptr->queue_data_ptr->start_index > queue_ptr->queue_data_ptr->end_index) ? 
                   (queue_ptr->queue_data_ptr->end_index + MAX_CAPACITY) : queue_ptr->queue_data_ptr->end_index;
    }
    
    for (int i = start_index,j=0; i < end_index; ++i, ++j) {
        printf("---- Element #%d ---\n", j);
        printf("Data: %s\n", queue_ptr->queue_data_ptr->array[i%MAX_CAPACITY].buffer_data);
    }
    printf(" -------------------------- \n");
}

int EnQueue(struct ProcessSafeQueue *queue_ptr, const void *data_ptr, 
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
    // int value = -1;
    // sem_post(queue_mutex_ptr);
    // sem_getvalue(queue_ptr->queue_mutex_ptr, &value);
    // printf("EnQueue sem value:%d\n", value);
    sem_wait(queue_ptr->queue_mutex_ptr);
    
    if (queue_ptr->queue_data_ptr->size < MAX_CAPACITY)
    {
        memcpy(queue_ptr->queue_data_ptr->array[queue_ptr->queue_data_ptr->end_index].buffer_data, 
               data_ptr, data_len);
        queue_ptr->queue_data_ptr->array[queue_ptr->queue_data_ptr->end_index].buffer_len = data_len;
        queue_ptr->queue_data_ptr->end_index++;
        if ((MAX_CAPACITY-1) < queue_ptr->queue_data_ptr->end_index)
        {
            queue_ptr->queue_data_ptr->end_index = 0;
        }
        queue_ptr->queue_data_ptr->size++;
        result = 0;
    } else {
        result = -1;
        perror("Overflow");
    }

    sem_post(queue_ptr->queue_mutex_ptr);

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
int DeQueue(struct ProcessSafeQueue *queue_ptr, void *data_ptr, 
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

    // int value = -1;
    // sem_wait(queue_mutex_ptr);
    // sem_getvalue(queue_ptr->queue_mutex_ptr, &value);
    // printf("DeQueue sem value:%d\n", value);
    sem_wait(queue_ptr->queue_mutex_ptr);
    
    if (queue_ptr->queue_data_ptr->size > 0)
    {
        memcpy(data_ptr, queue_ptr->queue_data_ptr->array[queue_ptr->queue_data_ptr->start_index].buffer_data, 
               queue_ptr->queue_data_ptr->array[queue_ptr->queue_data_ptr->start_index].buffer_len);
        *data_len = queue_ptr->queue_data_ptr->array[queue_ptr->queue_data_ptr->start_index].buffer_len;
        queue_ptr->queue_data_ptr->start_index++;

        if ((MAX_CAPACITY-1) < queue_ptr->queue_data_ptr->start_index)
        {
            queue_ptr->queue_data_ptr->start_index = 0;
        } 
        queue_ptr->queue_data_ptr->size--;
        result = 0;
    } else {
        result = -1;
        perror("Underflow");
    }

    sem_post(queue_ptr->queue_mutex_ptr);

    return result;
}

int WaitDeQueue(struct ProcessSafeQueue *queue_ptr, void *data_ptr, 
    uint32_t data_size, uint32_t *data_len, uint32_t timeout)
{
    int result = -1;
    struct timespec to;

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

    // int value = -1;
    // sem_wait(queue_mutex_ptr);
    // sem_getvalue(queue_ptr->queue_mutex_ptr, &value);
    // printf("DeQueue sem value:%d\n", value);

    to.tv_sec = time(NULL) + timeout;
    to.tv_nsec = 0;
    if (sem_timedwait (queue_ptr->queue_mutex_ptr, &to)) {
        result = -2;
        return result;
    }
    
    if (queue_ptr->queue_data_ptr->size > 0)
    {
        memcpy(data_ptr, queue_ptr->queue_data_ptr->array[queue_ptr->queue_data_ptr->start_index].buffer_data, 
               queue_ptr->queue_data_ptr->array[queue_ptr->queue_data_ptr->start_index].buffer_len);
        *data_len = queue_ptr->queue_data_ptr->array[queue_ptr->queue_data_ptr->start_index].buffer_len;
        queue_ptr->queue_data_ptr->start_index++;

        if ((MAX_CAPACITY-1) < queue_ptr->queue_data_ptr->start_index)
        {
            queue_ptr->queue_data_ptr->start_index = 0;
        } 
        queue_ptr->queue_data_ptr->size--;
        result = 0;
    } else {
        result = -1;
        perror("Underflow");
    }

    sem_post(queue_ptr->queue_mutex_ptr);

    return result;
}

int GetQueueSize(struct ProcessSafeQueue *queue_ptr)
{
    int result = 0;

    sem_wait(queue_ptr->queue_mutex_ptr);

    result = queue_ptr->queue_data_ptr->size;

    sem_post(queue_ptr->queue_mutex_ptr);

    return result;
}

int IsQueueEmpty(struct ProcessSafeQueue *queue_ptr)
{
    int result = 0;
    sem_wait(queue_ptr->queue_mutex_ptr);
    result = (queue_ptr->queue_data_ptr->size == 0) ? 1 : 0;
    sem_post(queue_ptr->queue_mutex_ptr);

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

    sem_close(queue_ptr->queue_mutex_ptr);

    sem_destroy(queue_ptr->queue_mutex_ptr);
}

/**
 * @brief detach process safe queue
 * 
 * @param queue_ptr 
 * @return int 
 */
int Detach(struct ProcessSafeQueue *queue_ptr) {
    return shmdt(queue_ptr->queue_data_ptr);
}