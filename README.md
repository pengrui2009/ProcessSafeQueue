# ProcessSafeQueue

 **process_safe_queue** is implemented as a library and offers really rich features. 
 
Repo Features :

- support multi processes send and receive msgs by queue. 
- support language: C/C++ .

# Process Safe Queue

This create an library for process safe queue. This queue can be used across multiple process inside a single system while
solving the receive and send message problem.

## Usage

### Default compilation

```
git clone git@github.com:pengrui2009/ProcessSafeQueue.git
cd process_safe_queue
```


```
sudo apt-get install libgtest-dev libgmock-dev
mkdir build
cd build
cmake ..
make all
```

It also contains `examples` and `units_test` :

- `examples`: contains the different process push and pop message.
- `units_test`: contains the unit test cases.

### Install it on your system

- Install it using:
    ```
    mkdir build
    cd build
    cmake .. && make all
    sudo make install
    ```
- Library name is: processSafeQueue
- Include file name  is: process_safe_queue.h
- If you want to use it in any of your c code do `#include <process_safe_queue.h>` and compile it using:
    ```
    # Here test.c is the name of your test file.
    gcc test.c -lprocessSafeQueue -lpthread -lrt
    ```
    
- If you want to use it in any of your c++ code do `#include <process_safe_queue.h>` and compile it using:
    ```
    # Here test.c is the name of your test file.
    g++ test.cpp -lprocessSafeQueue -lpthread -lrt
    ```

## Error Code:

- ERR_SYS             1          /* system fatal */
- ERR_SYS_SHMGET      10         /* system api:shmget fatal */
- ERR_SYS_SHMAT       11         /* system api:shmat fatal */
- ERR_SYS_SHMCTRL     12         /* system api:shmctrl fatal */
- ERR_SYS_SHMDT       13         /* system api:shmdt fatal */
- ERR_SYS_SEMOPEN     14         /* system api:semopen fatal */
- ERR_SYS_SEMWAIT     15         /* system api:semwait fatal */
- ERR_SYS_SEMTIMEWAIT 16         /* system api:semtimewait fatal */
- ERR_SYS_SEMPOST     17         /* system api:sempost fatal */
- ERR_SYS_SEMCLOSE    18         /* system api:semclose fatal */
- ERR_SYS_SEMDESTROY  19         /* system api:semdestroy fatal */

- ERR_QUEUE           2          /* queue fatal */
- ERR_QUEUE_UNDERFLOW 21         /* queue under flow fatal */
- ERR_QUEUE_OVERFLOW  22         /* queue over flow fatal */

- ERR_TIMEOUT         3          /* time out fatal */

- ERR_INVAL           4          /* paramters invalid fatal */
- ERR_INVAL_NULLPOINT 40         /* point is NULL */
- ERR_INVAL_MEMSIZE   41         /* params size  */

- ERR_NOINIT          5          /* no initialize fatal */
- ERR_NOMEM           6          /* no memory fatal */
- ERR_NODISK          7          /* no disk fatal*/
- ERR_CFG             8          /* config fatal */

- ERR_OTHER           80         /* others error */


## Docs

```
/**
 * Represents each element in the queue
 */
struct Element {
  uint8_t buffer_data[DATA_CAPACITY]; /* The data in the element*/
  uint32_t buffer_len;                /* The length in the element*/
};

struct Queue {
  int push_index;                    /*general queue push index*/
  int pop_index;                      /*general queue pop index*/
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
extern int Queue_Init(int id, struct ProcessSafeQueue* queue_ptr, int sync);

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

```
