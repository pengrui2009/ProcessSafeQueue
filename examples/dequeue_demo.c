#include "process_safe_queue.h"

#include <unistd.h>

int main()
{
    struct ProcessSafeQueue *queue_ptr = 
        (struct ProcessSafeQueue *)malloc(sizeof(struct ProcessSafeQueue)); 

    queue_ptr = InitQueue(560, queue_ptr, SYNC);
    sleep(1);
    PrintQueue(queue_ptr, ONLY_DATA);
    uint8_t buffer_data[30] = {0};
    uint32_t buffer_size = sizeof(buffer_data);
    uint32_t buffer_len = 0;

    if (Dequeue(queue_ptr, buffer_data, buffer_size, &buffer_len))
    {
        printf("Dequeue data: failed!\n");
    } else {
        printf("Dequeue data: success.\n");
    }

    PrintQueue(queue_ptr, ONLY_DATA);
    
    return 0;
}