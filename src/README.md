# Process Safe Queue

This create an library for process safe queue. This queue can be used across multiple process inside a single system while
solving the second reader writers problem.

## Usage

### Default compilation

```
make all
```

It also makes a binary called `test` that contains very basic usage of the code.
```
./test
```

### Install it on your system

- Install it using:
    ```
    sudo make install
    ```
- Library name is: safequeue
- Include file name  is: safe_queue.h
- If you want to use it in any of your code do `#include <safe_queue.h>` and compile it using:
    ```
    gcc test.c -lsafequeue -pthread
    ```
    Here test.c is the name of your.

## Testing

See Docs and edit test.c accordingly. Do make all. Execute ./test

## Error handled:
 - Underflow
 - Overflow
 - Re-creation of queue instead of sync


## Docs

```
struct Queue {
  int push_index;                     /*general queue push index*/
  int pop_index;                      /*general queue pop index*/
  int size;                           /*the size of queue element*/
  uint32_t buffer_size;               /*the capacity of one element buffer*/
  void *array[MAX_QUEUE_SIZE];        /*the array of buffer data structure to store all buffer contents
                                         in the queue*/
};

struct ProcessSafeQueue {
  key_t key_id;                       /* shm memory key id*/
  struct Queue* queue_data_ptr;       /* shm memory data ptr*/
  sem_t* queue_mutex_ptr;             /*Ensures atomicity of enqueue operation */
};
/**
 * Before using safe_queue you init get pointer to the queue data structure using this function.
 *
 * @param id - Unique identifier for the queue
 * @param q
 *
 * ueue - queue to be used in future
 * @param sync -
 * - CREAT:if using it for the first time.
 *      Creation will fail if a queue with the particular @param id has already been created
 * - SYNC: if the queue has already been created and you need a pointer to it
 * @return a pointer the safe_queue with @param id
 */
struct safe_queue *init_queue(int id, struct safe_queue *queue, int sync);

/**
 * Prints the content at index @param index of the safe_queue
 * @param queue pointer to the safe queue
 * @param index starts from start_index to end_index.
 * index whose content you want to be printed
 * @returns 0 if successfull else -1
 */
int queue_read_index(struct safe_queue *queue, int index);

/**
 * Writes the content, @param str, at index, @param index. of the safe_queue
 * @param queue pointer to the safe queue
 * @param index starts from start_index to end_index.index where you want to be writen
 * @param str the content you want to added
 * @returns 0 if successfull else -1
 */
int queue_write_index(struct safe_queue *queue, int index, char *str);

/**
 * Enqueues the content, str, in queue
 * @param queue
 * @param str The content that needs to be enqueued
 *  @returns 0 if successfull else -1 (Overflow)
 */
int queue_enqueue(struct safe_queue *queue, char *str);

/**
 * Dequeues an element from the queue
 * @param queue
 * @return The dequeued element's content or NULL in case of underflow
 */
char *queue_dequeue(struct safe_queue *queue);

/**
 * Prints the queue in a weird format
 * NOT INTER PROCESS PROCESS SAFE
 * @param queue
 * @param lock
 *  - EVERYTHING : Prints the state of each lock along with content in each element of queue
 *  - ONLY_DATA: Prints the content of the queue
 */
void print_queue(struct safe_queue *queue, int lock);


/**
 * Tries to destroy the queue.
 * NOT SAFE TO USE
 * @param queue a link to the queue
 * @param id the id of the queue
 */
void destroy(struct safe_queue *queue, int id);

/**
 * Detaches the current pointer of queue to the shared queue.
 * Coo?
 * @param queue
 * @return 0 if successful else * ERROR
 */
int detach(struct safe_queue *queue);
```
