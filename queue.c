#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <limits.h>
#include "queue.h"

#ifndef DEBUG
#define DEBUG 0
#endif
    
int createFQueue(struct Queue* q, char* fsize){
    q->front = NULL;
    q->rear = NULL;
    q->size = 0;
    q->open = 1;
    if(atoi(fsize) > 0){
        q->max = atoi(fsize);
    }
    else{
        return -1;
    }
    
    pthread_mutex_init(&q->lock, NULL);
    pthread_cond_init(&q->read_ready, NULL);
    pthread_cond_init(&q->write_ready, NULL);  
    return 0;
}
int createDQueue(struct Queue* q){
    q->front = NULL;
    q->rear = NULL;
    q->size = 0;
    q->open = 1;
    pthread_mutex_init(&q->lock, NULL);
    pthread_cond_init(&q->read_ready, NULL);
    pthread_cond_init(&q->write_ready, NULL);  
    return 0;
}

struct Qentry* create_entry(char* name){
    struct Qentry* temp = (struct Qentry*)malloc(sizeof(struct Qentry));
    int length = strlen(name) +1;
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

int dir_enqueue(Queue* q, char* name, int id){
    //printf("[%d] in dir_enqueue function\n",id);
    //printf("[%d] is waiting for the lock...\n",id);
    pthread_mutex_lock(&q->lock);
    //printf("[%d] Grabbed lock. Size of queue is %d\n",id,q->size);
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
    //printf("[%d] Unlocked\n",id);
    return 0;
}

int dir_dequeue(Queue* dq, struct Queue* fq, char** name, int* active, int id){
    //printf("[%d] in dir_dequeue function\n",id);
    //printf("[%d] is waiting for the lock...\n",id);
    pthread_mutex_lock(&dq->lock); //Grab lock
    //printf("[%d] Grabbed lock. Size of queue is %d\n",id,dq->size);
    if(dq->size == 0){
        //printf("[%d] Size of queue is 0!\n",id);
        *active = *active -1;
        //printf("[%d] Active thread count is now %d\n",id,*active);
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
        
    *active = *active + 1;
    //printf("[%d] queue size is %d. Active thread is now this %d\n", id, dq->size,*active);  
    }
    struct Qentry* pop = dq->front;
    dq->front = dq->front->next;
    dq->size--;
    if (dq->front == NULL){// If front becomes NULL, then change rear also as NULL
        dq->rear = NULL;
    }
    
    int length = strlen(pop->pathname)+1;
    char* name2 = (char*) malloc(length * sizeof(char));
    strcpy(name2, pop->pathname);
    *name = name2;
    //printf("[%d] Name of popped file is %s\n",id,name2);
    free(pop->pathname);
    free(pop);

    pthread_cond_signal(&dq->write_ready);
    pthread_mutex_unlock(&dq->lock);
    //printf("[%d] Unlocked lock. Size of queue is %d\n",id,dq->size);
    return 0;
}

int fil_enqueue(Queue* fq, char* name){
    pthread_mutex_lock(&fq->lock); //Grab lock
    
    while(fq->size >= fq->max){
        pthread_cond_wait(&fq->write_ready, &fq->lock);
    }
    if(fq->open == 0){ // the queue has been closed
        pthread_mutex_unlock(&fq->lock);
        return -1;
    }

    struct Qentry* add = create_entry(name);
    if (fq->front == NULL){
        fq->front = add;
        fq->rear = fq->front;
        fq->size++;
    }
    else{
        fq->rear->next = add;
        fq->rear = add;
        fq->size++;
    }
    
    pthread_cond_signal(&fq->read_ready);
    pthread_mutex_unlock(&fq->lock); // now we're done
    return 0; 
}

int fil_dequeue(struct Queue* dq, struct Queue* fq, char** name, int* active, int id){
   // printf("FQ[%d] in fil_dequeue function\n",id);
    //printf("FQ[%d] is waiting for the lock...\n",id);
    pthread_mutex_lock(&fq->lock); //Grab lock
    //printf("FQ[%d] has grabbed the lock...\n",id);

    if(fq->size == 0){
        //printf("FQ[%d] Size of queue is 0 relasing lock!\n",id);
        while(isEmpty(fq) && *active != 0){
            pthread_cond_wait(&fq->read_ready, &fq->lock);
        }

        if(*active == 0 && isEmpty(fq)){
            pthread_mutex_unlock(&fq->lock); 
            return -1;
        }
    //printf("FQ[%d] queue size is %d. Active thread is now this %d\n", id, dq->size,*active);
    }

    struct Qentry* pop = fq->front;
    fq->front = fq->front->next;
    fq->size--;
    
    if (fq->front == NULL)// If front becomes NULL, then change rear also as NULL
        fq->rear = NULL;

    int length = strlen(pop->pathname) +1;
    char* name2 = (char*) malloc(length * sizeof(char));
    strcpy(name2, pop->pathname);
    *name = name2;
    //printf("FQ[%d] Name of popped file is %s\n",id,name2);
    free(pop->pathname);
    free(pop);

    pthread_cond_signal(&dq->write_ready);
    pthread_mutex_unlock(&fq->lock);
    //printf("FQ[%d] Unlocked lock. Size of queue is %d\n",id,dq->size);
    return 0;
}
