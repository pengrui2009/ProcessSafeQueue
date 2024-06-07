#include "process_safe_queue.h"
#include <unistd.h>

int main()
{
    struct ProcessSafeQueue *queue_ptr = 
        (struct ProcessSafeQueue *)malloc(sizeof(struct ProcessSafeQueue)); 

    queue_ptr = InitQueue(560, queue_ptr, SYNC);

    PrintQueue(queue_ptr, ONLY_DATA);
    uint8_t *buffer_data = "first queue";
    uint32_t buffer_len = strlen(buffer_data);

    if (Enqueue(queue_ptr, buffer_data, buffer_len))
    {
        printf("Enqueue data: failed!\n");
    } else {
        printf("Enqueue data: success.\n");
    }

    PrintQueue(queue_ptr, ONLY_DATA);
    
    return 0;
}