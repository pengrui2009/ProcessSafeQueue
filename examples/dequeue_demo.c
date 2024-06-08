#include "process_safe_queue.h"

#include <unistd.h>

int main()
{
    // struct ProcessSafeQueue *queue_ptr = 
    //     (struct ProcessSafeQueue *)malloc(sizeof(struct ProcessSafeQueue)); 
    struct ProcessSafeQueue queue;
    if (InitQueue(560, &queue, SYNC))
    {
        printf("InitQueue failed!\n");
        return -1;
    }
    
    PrintQueue(&queue, ONLY_DATA);
    uint8_t buffer_data[30] = {0};
    uint32_t buffer_size = sizeof(buffer_data);
    uint32_t buffer_len = 0;
    sleep(1);
    if (DeQueue(&queue, buffer_data, buffer_size, &buffer_len))
    {
        printf("Dequeue data: failed!\n");
    } else {
        printf("Dequeue data: %s.\n", (char *)buffer_data);
    }

    PrintQueue(&queue, ONLY_DATA);
    
    return 0;
}