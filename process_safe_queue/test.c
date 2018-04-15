
#include "safe_queue.h"

int main(int argc, char const *argv[]) {
    struct safe_queue *queue = (struct safe_queue *) malloc(sizeof(struct safe_queue));
    queue = init_queue(560, queue, SYNC);


    print_queue(queue, ONLY_DATA);

    queue_enqueue(queue, "first enqueue");
    print_queue(queue, ONLY_DATA);

    queue_enqueue(queue, "second enqueue");
    print_queue(queue, ONLY_DATA);

    queue_dequeue(queue);
    print_queue(queue, ONLY_DATA);

    return 0;
}