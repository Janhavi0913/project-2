#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "queue.c"
//#include "file.c"

struct variables{
    struct Queue* filequ;
    struct Queue* dirqu;
    int thread_id;
    //struct wordnode* filelist;
    int *active;
}variables;

int d_thread = 3, f_thread = 1, a_thread = 1;
char *suf = ".txt";

int op_args (char* input){
    if((input[1] != 'd') || (input[1] != 'f') || (input[1] != 'a') || (input[1] != 's') ){
            return 1;
    }
    if(isdigit(input[2])){
        char* temp1;

        for(int p = 2; p < strlen(input); p++){
            temp1+=input[p];
        }
        if(atoi(temp1) <= 0){
            return 1;
        }
        if(input[1] == 'd'){
            d_thread = atoi(temp1);
        } 
        if(input[1] == 'f'){
            f_thread = atoi(temp1);
        }
        if(input[1] == 'a'){
            a_thread = atoi(temp1);
        }
        free(temp1);
        return 0;
    }
    if(input[1] == 's'){
        char* temp2; 
        for(int p = 2; p < strlen(input); p++){
            temp2 += input[p];
        }
        strcpy(suf, temp2);
        free(temp2);
        return 0;
    }
    else
        return 1; 
}

int isdir(char *name) {
    struct stat data;
	int err = stat(name, &data);
	if (err){ 
		perror(name);
		return 0;
	}
	if (S_ISDIR(data.st_mode)){ // S_ISDIR macro is true if the st_mode says the file is a directory
		return 0;
	} 
	return 1;
}

int check_suffix(char* filename){
    int fnl = strlen(filename);
    int sufl = strlen(suf);
    int dif = fnl - sufl;
    if(fnl >= sufl){
        if(strcmp(filename + dif, suf) == 0){
            return 1;
        }
    }
    return 0;
}
void destroy_lock(Queue* dq){
    pthread_mutex_destroy(&dq->lock);
    pthread_cond_destroy(&dq->read_ready);
    pthread_cond_destroy(&dq->write_ready); 
}
void* directory_traverse(void *A){
    struct variables *var = A;
    char* curdir = NULL;
    printf("[%d] Thread is here\n",var->thread_id);
    int proceed = dir_dequeue(var->dirqu, var->filequ, &curdir, var->active, var->thread_id);
	if(proceed != 0){ // no work is needed to be done exit the function and join all threads
        printf("[%d] Exiting...\n",var->thread_id);
        pthread_exit(NULL);
	}
    printf("[%d]]Processing file %s\n",var->thread_id,curdir);

    DIR *pdir = opendir(curdir);
    struct dirent *entries;
    if(pdir == NULL){
        perror("Directory cannot be open");
    }
    while((entries = readdir(pdir)) != NULL){
        if(!strcmp(".",entries->d_name) || !strcmp("..",entries->d_name)){
            continue;
        }
        else{
            char* fname = entries->d_name;
            int length = strlen(curdir) + strlen(fname) + 2;
            char* pathname = (char*) malloc(length * sizeof(char));
            strcpy(pathname,curdir);
            strcat(pathname,"/");
            strcat(pathname,entries->d_name);
            if(isdir(pathname) == 1){ // this is a file 
                if(check_suffix(pathname) == 1){ // add to file queue if the suffix match
                   fil_enqueue(var->filequ, pathname);
                }
            }  
            else{ // this is a directory add to directory queue
                dir_enqueue(var->dirqu, pathname, var->thread_id);
            }
        free(pathname);
        }
    }
    closedir(pdir);
    free(curdir);
    
    directory_traverse(var);
}

int main(int argc, char **argv){
    if(argc < 2){ 
        perror("Number of argument error\n");
        return EXIT_FAILURE;
    }

    struct Queue fq;
    struct Queue dq;
    struct variables *data;
    pthread_t *dir_tid;

    for(int m = 1; m < argc; m++){ // traverse optional arguments
        int length = strlen(argv[m])+1;
        char* input = (char*) malloc(length * sizeof(char));
        strcpy(input, argv[m]);
        if(input[0] == '-'){ // optional argument
            int error = op_args(input);
            if(error == 1){
                perror("Incorrect Optional Argument");
                return EXIT_FAILURE;
            }
        }
        free(input);
    }

    createFQueue(&fq,"1000");
    createDQueue(&dq);
	
    // TODO: start file threads 
	
    for(int m = 1; m < argc; m++){ // traverse through arguments get directory and files
        int length = strlen(argv[m])+1;
        char* input = (char*) malloc(length * sizeof(char));
        strcpy(input, argv[m]);
        if(input[0] == '-'){
            free(input);// optional argument - ignore
            continue;
        }
        else{
            if(isdir(input) == 1){ // file add to file queue
               fil_enqueue(&fq, input);
            }
            else{ // directory add to directory queue
              dir_enqueue(&dq, input, 4);
            }
        }
        free(input);
    }
       
    int err2, active = d_thread;
    data = malloc(d_thread * sizeof(struct variables));
    dir_tid = malloc(d_thread * sizeof(pthread_t));
    for(int m = 0; m < d_thread; m++){
	    data[m].filequ = &fq;
	    data[m].dirqu = &dq;
        data[m].thread_id = m;
	    data[m].active = &active;
        err2 = pthread_create(&dir_tid[m], NULL, directory_traverse, &data[m]);
        printf("thread id %ld\n", dir_tid[m]);
        if(err2 != 0){
            perror("pthread_create");
            abort();
        }
    }
    printf("main is continue\n");

    for(int m = 0; m < d_thread; m++){
        err2 = pthread_join(dir_tid[m], NULL);
    }
    printf("threads are done\n");

    destroy_lock(data->dirqu);
    free(data);
    free(dir_tid);

    // TODO: start analysis threads and phase 2 [JSD]
    
    return EXIT_SUCCESS;
}
