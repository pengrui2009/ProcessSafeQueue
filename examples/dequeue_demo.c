#include "process_safe_queue.h"

#include <unistd.h>

int main()
{
    int result = -1;
    // struct ProcessSafeQueue *queue_ptr = 
    //     (struct ProcessSafeQueue *)malloc(sizeof(struct ProcessSafeQueue)); 
    struct ProcessSafeQueue queue;
    result = Queue_Init(562, &queue, 100);
    if (result)
    {
        printf("Queue_Init failed, result=%d!\n", result);
        return -1;
    }
    
    Queue_Print(&queue);
    uint8_t buffer_data[100] = {0};
    uint32_t buffer_size = sizeof(buffer_data);
    uint32_t buffer_len = 0;
    sleep(1);

    result = Queue_Pop(&queue, buffer_data, buffer_size, &buffer_len);
    if (result)
    {
        printf("Dequeue data: failed result=%d!\n", result);
    } else {
        printf("Dequeue data: %s.\n", (char *)buffer_data);
    }

    Queue_Print(&queue);
    
    return 0;
}