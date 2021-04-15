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
#include "file.c"

struct variables{
    struct Queue* filequ;
    struct Queue* dirqu;
    int thread_id;
    struct filenode* filelist;
    pthread_mutex_t *lock;
    int *active;
}variables;

int d_thread = 1, f_thread = 2, a_thread = 1;
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
    while(1){
    struct variables *var = A;
    char* curdir = NULL;
    int proceed = dir_dequeue(var->dirqu, var->filequ, &curdir, var->active, var->thread_id);
	if(proceed != 0){ // no work is needed to be done exit the function and join all threads
        pthread_exit(NULL);
	}
   
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
    }

}

void* file_traverse(void *A){
	struct variables *var = (struct variables *)A;
    printf("[%d] FILE Thread is here\n",var->thread_id);
	while((var->active != 0) || (isEmpty(var->filequ) == 0)){
		char *curfile = NULL;
		int proceed = fil_dequeue(var->dirqu, var->filequ, &curfile, var->active, var->thread_id);
		if(proceed != 0){
            printf("[%d]FILE Exiting...\n",var->thread_id);
			pthread_exit(NULL);
		}
        printf("[%d]] FILE Processing file %s\n",var->thread_id,curfile);
		int fd = open(curfile, O_RDONLY);
			if(fd == -1)
				perror(curfile);

		strbuf_t file = readFile(fd);

		int i = 0;
		char delim[2] = " ";
		char* str = strtok(file.data, delim);
		wordnode* head = insert(NULL, str);
		i += strlen(str);

		while(i < file.length){
			str = strtok(NULL, delim);
			if(str == NULL)
				break;
			head = insert(head, str);
			i += strlen(str);
		}

		sb_destroy(&file);

		wordnode* ptr = head;

		while(ptr != NULL){
			ptr->WFD = ptr->numoccur/head->totalnodes;
			ptr = ptr->next;
		}

		addToFileList(var->filelist, curfile, head, var->thread_id, var->lock); // this will call createfilenode

		//freeNodes(head);
	}
}

int main(int argc, char **argv){
    if(argc < 2){ 
        perror("Number of argument error\n");
        return EXIT_FAILURE;
    }

    struct Queue fq;
    struct Queue dq;
    struct filenode *fl = (struct filenode*)malloc(sizeof(struct filenode));
    fl->filename = NULL;
    struct variables *data;
    pthread_t *tids;
    pthread_mutex_t file_lock;

    // traverse optional arguments
    for(int m = 1; m < argc; m++){ 
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

    int total_threads = f_thread + d_thread;
    createFQueue(&fq,"1000");
    createDQueue(&dq);
    pthread_mutex_init(&file_lock, NULL);

    int p, error, active = d_thread;
    data = malloc(total_threads * sizeof(struct variables));
    tids = malloc(total_threads * sizeof(pthread_t));

    // start file threads
    for(p = 0; p < f_thread; p++){
	    data[p].filequ = &fq;
	    data[p].dirqu = &dq;
        data[p].thread_id = p;
        data[p].filelist = fl;
        data[p].lock = &file_lock;
	    data[p].active = &active;
        error = pthread_create(&tids[p], NULL, file_traverse, &data[p]);
        printf("thread id %ld\n", tids[p]);
        if(error != 0){
            perror("pthread_create");
            abort();
        }
    } 
	
    // traverse arguments for directory and files
    for(int m = 1; m < argc; m++){ 
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
              dir_enqueue(&dq, input, -1);
            }
        }
        free(input);
    }

    // start directory threads
    for(; p < total_threads; p++){
	    data[p].filequ = &fq;
	    data[p].dirqu = &dq;
        data[p].thread_id = p;
        data[p].filelist = fl;
	    data[p].active = &active;
        error = pthread_create(&tids[p], NULL, directory_traverse, &data[p]);
        printf("thread id %ld\n", tids[p]);
        if(error != 0){
            perror("pthread_create");
            abort();
        }
    }
    printf("main is continue\n");

    // TODO: start analysis threads phase 2 [JSD]

    for(int m = 0; m < total_threads; m++){
        pthread_join(tids[m], NULL);
    }
    printf("threads are done\n");

    printf("[0]Name of file is %s\n", data[0].filelist->filename);
    //printf("Name of file is %d\n", data[0].filelist->totalnodes);
    printf("[1]Name of file is %s\n", data[0].filelist->next->filename);
    //printf("Name of file is %s\n", data[1].filelist->filename);

    destroy_lock(data->dirqu);
    destroy_lock(data->filequ);
    free(data);
    free(tids);

    return EXIT_SUCCESS;
}
