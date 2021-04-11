#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <limits.h>
#include "queue.h"


#ifndef DEBUG
#define DEBUG 0
#endif
    
struct Queue* createQueue(){
    struct Queue* q = (struct Queue*)malloc(sizeof(struct Queue));
    q->front = NULL;
    q->rear = NULL;
    q->size = 0;
    q->open = 1;
    pthread_mutex_init(&q->lock, NULL);
    pthread_cond_init(&q->read_ready, NULL);
    pthread_cond_init(&q->write_ready, NULL);  
    return q;
}

struct Qentry* create_entry(char* name){
    struct Qentry* temp = (struct Qentry*)malloc(sizeof(struct Qentry));
    int length = strlen(name);
    temp->pathname = (char*) malloc(length * sizeof(char));
    strcpy(temp->pathname, name);
    temp->next = NULL;
    return temp;
} 

int isEmpty(struct Queue* q){
    if(q->size == 0){
        return 0;
    }
return 1;        
}

void enqueue(struct Queue* q, char* name){
    pthread_mutex_lock(&q->lock);
    if(q->open == 0){
        pthread_mutex_unlock(&q->lock);
    }
    struct Qentry* add = create_entry(name);
    if (q->front == NULL) {
        q->front = add;
        q->rear = q->front;
        q->size++;
        return;
    }
    q->rear->next = add;
    q->rear = add;
    q->size++;
    pthread_mutex_unlock(&q->lock); // now we're done
    return;
}
  
char* dequeue(struct Queue* q){
    pthread_mutex_lock(&q->lock);
    while ((q->open == 1) && (q->size == 0)) {
        pthread_cond_wait(&q->read_ready, &q->lock);
    }
    if(q->open == 0){
        pthread_mutex_unlock(&q->lock);
    }
    struct Qentry* pop = q->front;
    q->front = q->front->next;
    q->size--;
    
    if (q->front == NULL)// If front becomes NULL, then change rear also as NULL
        q->rear = NULL;

    int length = strlen(pop->pathname);
    char* pathname = (char*) malloc(length * sizeof(char));
    strcpy(pathname, pop->pathname);
    free(pop->pathname);
    free(pop);
    /* pthread_mutex_unlock(&q->lock); // now we're done
    pthread_cond_signal(&q->read_ready); // wake up a thread waiting to read (if any) */
    return pathname;
}

int close_queue(struct Queue* q){
    pthread_mutex_lock(&q->lock);
    q->open = 0;
    pthread_mutex_unlock(&q->lock);
}
