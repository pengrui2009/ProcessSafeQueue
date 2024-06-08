#include "process_safe_queue.h"
#include <unistd.h>

int main(int argc, char *argv[])
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
    uint8_t buffer_data[50] = {0};
    uint32_t buffer_len = strlen(buffer_data);
    if (argc >=2 )
    {
        sprintf(buffer_data, "this is queue element %s", argv[1]);
    } else {
        sprintf(buffer_data, "this is queue element0");
    }
    sleep(1);
    

    if (EnQueue(&queue, buffer_data, buffer_len))
    {
        printf("Enqueue data: failed!\n");
    } else {
        printf("Enqueue data: success.\n");
    }

    PrintQueue(&queue, ONLY_DATA);
    
    return 0;
}