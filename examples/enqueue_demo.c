#include "process_safe_queue.h"
#include <unistd.h>

int main(int argc, char *argv[])
{

    // struct ProcessSafeQueue *queue_ptr = 
    //     (struct ProcessSafeQueue *)malloc(sizeof(struct ProcessSafeQueue)); 
    struct ProcessSafeQueue *queue_ptr = NULL;
    if (InitQueue(580, &queue_ptr, SYNC))
    {
        printf("InitQueue failed!\n");
        return -1;
    }

    PrintQueue(queue_ptr, ONLY_DATA);
    uint8_t buffer_data[50] = {0};
    if (argc >=2 )
    {
        sprintf(buffer_data, "this is queue element %s", argv[1]);
    } else {
        sprintf(buffer_data, "this is queue element0");
    }
    
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