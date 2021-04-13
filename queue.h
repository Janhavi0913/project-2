typedef struct Qentry {
    char* pathname;
    struct Qentry* next;
}Qentry;
  
typedef struct Queue {
    struct Qentry *front;
    struct Qentry *rear;
    int size;
    int open;
    int max;
    pthread_mutex_t lock;
    pthread_cond_t read_ready;
    pthread_cond_t write_ready;
}Queue;

struct Queue* create_Queue(int size); // bounded
struct Qentry* create_entry(char* name);
void close_queue(struct Queue* q);
int isEmpty(struct Queue* q);
int dir_enqueue(struct Queue* q, char* name);
int dir_dequeue(struct Queue* dq, struct Queue* fq, char* name, int* active);
int fil_enqueue(struct Queue* q, char* name);
int fil_dequeue(struct Queue* dq, struct Queue* fq, char* name, int* active);

/*
if number of active is not 0 AND the queue is empty we want to still wait to see if the q ends up having something
if the q is empty and there is no active threads we want to terminate
if the number of active is not 0 but the q is not empty we want to continue processing
if the q is not empty and there are no active threads we want to keep processing
  
int dequeue(struct Queue* q, char* name, int* active){
    pthread_mutex_lock(&q->lock);
    while ((q->open == 1) && (q->size == 0)){
        pthread_cond_wait(&q->read_ready, &q->lock);
    }
    if(q->open == 0){
        pthread_mutex_unlock(&q->lock);
        return -1;
    }
    struct Qentry* pop = q->front;
    q->front = q->front->next;
    q->size--;
    
    if (q->front == NULL)// If front becomes NULL, then change rear also as NULL
        q->rear = NULL;

    int length = strlen(pop->pathname);
    *name = (char*) malloc(length * sizeof(char));
    strcpy(name, pop->pathname);
    free(pop->pathname);
    free(pop);
    pthread_cond_signal(&q->write_ready);
    pthread_mutex_unlock(&q->lock); // now we're done
    return 0;
}

*/