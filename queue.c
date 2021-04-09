#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <pthread.h>
#include "queue.h"

#ifndef DEBUG
#define DEBUG 0
#endif

void queue_init(struct Queue *Q){
    Q = (struct Queue*)malloc(sizeof(struct Queue));
    Q->front = 0;
    Q->size = 0;
    Q->rear = Q->size - 1;
    char **pathname;
    pthread_mutex_init(&Q->lock, NULL);
    pthread_cond_init(&Q->read_ready, NULL);
    pthread_cond_init(&Q->write_ready, NULL);    
    //return err;  // obtained from the init functions (code omitted)
}

void enqueue(struct Queue *Q, char *name){
    pthread_mutex_lock(&Q->lock); // make sure no one else touches Q until we're done
    /* while (Q->size == QUEUESIZE) {  
        pthread_cond_wait(&Q->write_ready, &Q->lock); // wait for another thread to dequeue
            // release lock & wait for a thread to signal write_ready
    } */
    // at this point, we hold the lock & Q->count < QUEUESIZE
    unsigned index = Q->front + Q->size;
    /* if (index >= QUEUESIZE) 
        index -= QUEUESIZE; */
    Q->pathname[index] = (char*)malloc((sizeof(name + 1))* sizeof(char));
    Q->size++; 
    pthread_mutex_unlock(&Q->lock); // now we're done
    pthread_cond_signal(&Q->read_ready); // wake up a thread waiting to read (if any)

    //return 0;
}

char* dequeue(struct Queue *Q){
    pthread_mutex_lock(&Q->lock);
    while (Q->size == 0) {
        pthread_cond_wait(&Q->read_ready, &Q->lock);
    }
        // now we have exclusive access and queue is non-empty
    char *pathname = Q->pathname[Q->front];  // write value at head to pointer
    Q->size--;
    Q->front ++;
    /* if (Q->front == QUEUESIZE) 
        Q->front = 0; */
    pthread_mutex_unlock(&Q->lock);
    pthread_cond_signal(&Q->write_ready);
    return pathname;
}

int isEmpty(struct Queue *Q){
    if(Q->size == 0){
        return 0;
    }
return 1;
}
