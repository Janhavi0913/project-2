#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <limits.h>
#include "queue.h"

#ifndef DEBUG
#define DEBUG 0
#endif
    
struct Queue* createQueue(int size){
    struct Queue* q = (struct Queue*)malloc(sizeof(struct Queue));
    q->front = NULL;
    q->rear = NULL;
    q->size = 0;
    q->open = 1;
    if(size != NULL){
        q->max = size;
    }
    else{
        q->max = NULL;
    }
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

void close_queue(struct Queue* q){
    pthread_mutex_lock(&q->lock);
    q->open = 0;
    pthread_cond_broadcast(&q->read_ready);
    pthread_cond_broadcast(&q->write_ready);
    pthread_mutex_unlock(&q->lock);
}

int isEmpty(struct Queue* q){
    if(q->size == 0){
        return 1;
    }
return 0;
}

int dir_enqueue(struct Queue* q, char* name){
    pthread_mutex_lock(&q->lock);
    if(q->open == 0){ // the queue has been closed
        pthread_mutex_unlock(&q->lock);
        return -1;
    }
    struct Qentry* add = create_entry(name);
    if (q->front == NULL) {
        q->front = add;
        q->rear = q->front;
        q->size++;
    }
    else{
        q->rear->next = add;
        q->rear = add;
        q->size++;
    }
    
    pthread_cond_signal(&q->read_ready);
    pthread_mutex_unlock(&q->lock); // now we're done
    return 0;
}

int directory_dequeue(struct Queue* dq, struct Queue* fq, char* name, int* active){
    pthread_mutex_lock(&dq->lock);//Grab lock

    if(dq->size == 0){
        *active--;
        if(*active == 0){            
            pthread_cond_broadcast(&dq->write_ready); //wake up all threads
            pthread_cond_broadcast(&dq->read_ready);
            pthread_cond_broadcast(&fq->read_ready);
            pthread_mutex_unlock(&dq->lock); 
            return -1;
        }
        
        while(isEmpty(dq) && *active != 0){
            pthread_cond_wait(&dq->read_ready, &dq->lock); 
        }

        if(*active == 0){
            pthread_mutex_unlock(&dq->lock); 
            return -1;
        }
    *active++;
    }

    struct Qentry* pop = dq->front;
    dq->front = dq->front->next;
    dq->size--;
    if (dq->front == NULL){// If front becomes NULL, then change rear also as NULL
        dq->rear = NULL;
    }
    int length = strlen(pop->pathname);
    name = (char*) malloc(length * sizeof(char));
    strcpy(name, pop->pathname);
    free(pop->pathname);
    free(pop);

    pthread_cond_signal(&dq->write_ready);
    pthread_mutex_unlock(&dq->lock); // now we're done
    return 0;
}

int file_enqueue(struct Queue* fq, char* name){
    pthread_mutex_lock(&fq->lock); //Grab lock
    
    while(fq->size >= fq->max){
        pthread_cond_wait(&fq->write_ready, &fq->lock);
    }
    if(fq->open == 0){ // the queue has been closed
        pthread_mutex_unlock(&q->lock);
        return -1;
    }

    struct Qentry* add = create_entry(name);
    if (q->front == NULL){
        q->front = add;
        q->rear = q->front;
        q->size++;
    }
    else{
        q->rear->next = add;
        q->rear = add;
        q->size++;
    }
    
    pthread_cond_signal(&fq->read_ready);
    pthread_mutex_unlock(&fq->lock); // now we're done
    return 0; 
}

int file_dequeue(struct Queue* dq, struct Queue* fq, char* name, int* active){
    pthread_mutex_lock(&fq->lock);//Grab lock

    if(fq->size == 0){
        while(isEmpty(fq) && *active != 0){
            //release lock, wait for read ready then grab lock again
            pthread_cond_wait(&fq->read_ready, &fq->lock);
        }
        if(*active == 0){
            pthread_mutex_unlock(&fq->lock); 
            return -1;
        }
    *active++;
    }

    struct Qentry* pop = fq->front;
    fq->front = fq->front->next;
    fq->size--;
    
    if (fq->front == NULL)// If front becomes NULL, then change rear also as NULL
        fq->rear = NULL;

    int length = strlen(pop->pathname);
    *name = (char*) malloc(length * sizeof(char));
    strcpy(name, pop->pathname);
    free(pop->pathname);
    free(pop);

    pthread_cond_signal(&dq->write_ready);
    pthread_mutex_unlock(&fq->lock); // now we're done
    return 0;
}