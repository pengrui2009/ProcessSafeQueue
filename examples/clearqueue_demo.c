#include "process_safe_queue.h"
#include <unistd.h>

int main(int argc, char *argv[])
{
    int result = -1;
    // struct ProcessSafeQueue *queue_ptr = 
    //     (struct ProcessSafeQueue *)malloc(sizeof(struct ProcessSafeQueue)); 
    struct ProcessSafeQueue queue;
    result = Queue_Init(569, &queue, 100);
    if (result)
    {
        printf("InitQueue failed, result=%d!\n", result);
        return -1;
    }

    Queue_Print(&queue);

    result = Queue_Deinit(&queue);
    if (result)
    {
        printf("Queue_Deinit failed, result=%d!\n", result);
        return -1;
    }
    
    return 0;
}