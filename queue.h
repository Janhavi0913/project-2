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

int createFQueue(struct Queue* fq,char* size); // bounded
int createDQueue(struct Queue* dq); // unbounded
struct Qentry* create_entry(char* name);
void close_queue(struct Queue* q);
int isEmpty(struct Queue* q);
int dir_enqueue(Queue* q, char* name, int id);
int dir_dequeue(struct Queue* dq, struct Queue* fq, char** name, int* active, int id);

int fil_enqueue(Queue* q, char* name);
int fil_dequeue(struct Queue* dq, struct Queue* fq, char** name, int* active, int id);
