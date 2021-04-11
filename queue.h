typedef struct Qentry {
    char* pathname;
    struct Qentry* next;
}Qentry;
  
typedef struct Queue {
    struct Qentry *front;
    struct Qentry *rear;
    int size;
    int open;
    pthread_mutex_t lock;
    pthread_cond_t read_ready;
    pthread_cond_t write_ready;
}Queue;

struct Qentry* create_entry(char* name);
struct Queue* createQueue();
void enqueue(struct Queue* q, char* name);
char* dequeue(struct Queue* q);
int isEmpty(struct Queue* q);

