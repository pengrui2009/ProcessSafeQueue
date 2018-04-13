#define MAX_CAPACITY 3

struct lock_custom {
	sem_t * reader;
	sem_t * writer;
	sem_t * mut;
};

struct element {
	char *data;
	struct lock_custom lock;
};

struct lame_queue {
	int start_index;
	int end_index;
	sem_t * enqueue_muttex;
	sem_t * dequeue_muttex;
	struct element array[MAX_CAPACITY];
};


struct lame_queue *init_queue(int id);

struct lame_queue *sync_queue(int id,  struct lame_queue *queue);

void queue_read_index(struct lame_queue *queue, int index);

void queue_write_index(struct lame_queue *queue, int index, char *str);

void queue_enqueue(struct lame_queue *queue, char *str);

char * queue_dequeue(struct lame_queue *queue);
