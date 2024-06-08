#include "process_safe_queue.h"
#include <unistd.h>

int main(int argc, char *argv[])
{
    int result = -1;
    // struct ProcessSafeQueue *queue_ptr = 
    //     (struct ProcessSafeQueue *)malloc(sizeof(struct ProcessSafeQueue)); 
    struct ProcessSafeQueue queue;
    result = Queue_Init(562, &queue, 100);
    if (result)
    {
        printf("InitQueue failed, result=%d!\n", result);
        return -1;
    }

    Queue_Print(&queue);
    uint8_t buffer_data[50] = {0};
    
    if (argc >=2 )
    {
        sprintf(buffer_data, "this is queue element %s", argv[1]);
    } else {
        sprintf(buffer_data, "this is queue element0");
    }
    uint32_t buffer_len = strlen(buffer_data);
    sleep(1);
    
    result = Queue_Push(&queue, buffer_data, buffer_len);
    if (result)
    {
        printf("Enqueue data: failed, result=%d!\n", result);
    } else {
        printf("Enqueue data: success.\n");
    }

    Queue_Print(&queue);
    
    return 0;
}