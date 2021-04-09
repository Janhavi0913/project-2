typedef struct Queue{
    int front, rear, size;
    char **pathname;
    pthread_mutex_t lock;
    pthread_cond_t read_ready;  // wait for count > 0
    pthread_cond_t write_ready; // wait for count < QUEUESIZE
} Queue;

void queue_init(struct Queue *Q);
void enqueue(struct Queue* Q, char *pathname);
char* dequeue(struct Queue* Q);
int isEmpty(struct Queue *Q);

