#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "strbuf.c"
#include "queue.c"
#include "file.c"
#include "analysis.c"

struct variables{
    struct Queue* filequ;
    struct Queue* dirqu;
    int thread_id;
    struct filenode* filelist;
	struct comp_result* results;
    int start;
    int end;
    pthread_mutex_t *lock;
    int *active;
}variables;

int d_thread = 1, f_thread = 1, a_thread = 1;
char *suf = ".txt";

int op_args (char* input){
    if((input[1] != 'd') && (input[1] != 'f') && (input[1] != 'a') && (input[1] != 's') ){
            return 1;
    }
    if(input[1] == 's'){
        char* temp2 = calloc( (strlen(input)) , sizeof(char));
        memcpy(temp2, &input[2],(strlen(input)-2));
        temp2[(strlen(input)-1)] = '\0';
    
        suf = temp2;
        return 0;
    }
        char* temp1 = calloc( (strlen(input)) , sizeof(char));
        memcpy(temp1, &input[2],(strlen(input)-2));
        temp1[(strlen(input)-1)] = '\0';

        if(temp1 == NULL){
            return 1;
        }
        
        if(atoi(temp1) <= 0){
            free(temp1);
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
       return 0; 
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
void freeFileNodes(filenode* head){
    if(head != NULL)
        return;
    freeFileNodes(head->next);
    freeNodes(head->head);
    free(head);
}
void* directory_traverse(void *A){
    while(1){
    struct variables *var = A;
    char* curdir = NULL;
    int proceed = dir_dequeue(var->dirqu, var->filequ, &curdir, var->active, var->thread_id);
	if(proceed != 0){ // no work is needed to be done exit the function and join all threads
        printf("[%d]DIR Exiting...\n",var->thread_id);
        pthread_exit(NULL);
	}
   
    DIR *pdir = opendir(curdir);
    struct dirent *entries;
    if(pdir == NULL){
        free(curdir);
        perror("Directory cannot be open");
        continue;
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
    printf("[%d] FILE Thread the fl count we are seeing is %d\n",var->thread_id,*var->filelist->total_files);
	while((var->active != 0) || (isEmpty(var->filequ) == 0)){
		char *curfile = NULL;
		int proceed = fil_dequeue(var->dirqu, var->filequ, &curfile, var->active, var->thread_id);
		if(proceed != 0){
            printf("[%d]FILE Exiting...\n",var->thread_id);
			pthread_exit(NULL);
		}
        printf("[%d]] FILE Processing file %s\n",var->thread_id,curfile);
		int fd = open(curfile, O_RDONLY);
			if(fd == -1){
                perror(curfile);
                continue;
            }
				
		strbuf_t file = readFile(fd);

		int i = 0;
		char delim[2] = " ";
		char* str;
        str = strtok(file.data, delim);
        printf("the word is %s\n ", str);
		wordnode* head = insert(NULL, str);
        
		i += strlen(str);

		while(i < file.length){
			str = strtok(NULL, delim);
			if(str == NULL)
				break;
			head = insert(head, str);
            //printf("In the while loop word in head is %s\n", head->word);
			i += strlen(str);
		}

		

		wordnode* ptr = head;
        //printf("Before pointer look this is the word in head is %s\n", head->word);
		while(ptr != NULL){
			ptr->WFD = (double)ptr->numoccur/(double)head->totalnodes;
			ptr = ptr->next;
		}
        printf(" After pointer look this is the word in head is %s\n", head->word);
		addToFileList(var->filelist, curfile, head, var->thread_id, var->lock); // this will call createfilenode
        sb_destroy(&file);
		//freeNodes(head);
	}
}

void* analysis(void *A)
{
    struct variables *var = (struct variables *)A;
    printf("file 1 is %s\n",var->results[0].file1);
    printf("file 2 is %s\n",var->results[0].file2);
    printf("[%d] Analysis Thread\n",var->thread_id);
    int i;
    printf("the value of var start: %d and var end %d\n", var->start, var->end);
    for(i = var->start; i < var->end; i++)
    {
        char *file1, *file2;
	    file1 = var->results[i].file1;
        printf("file 1 is %s\n",var->results[i].file1);
	    file2 = var->results[i].file2;
        printf("file 2 is %s\n",var->results[i].file2);
        filenode* f1 = NULL;
        filenode* f2 = NULL;
        filenode* ptr = var->filelist;
        if(f1 == NULL){
            printf("F1 IS NULL\n");
        } else {
            printf("F1 IS NOT NULL\n");
        }

        while(ptr != NULL){
            if(strcmp(ptr->filename, file1) == 0)
                f1 = ptr;
            else if(strcmp(ptr->filename, file2) == 0)
                f2 = ptr;
            if(f1 != NULL && f2 != NULL){
                break;
            }
            ptr = ptr->next;
        }

        printf("FINISHED FINDING FILES\n");
         printf("file 1 is %s\n",f1->filename);
        printf("file 2 is %s\n",f2->filename);
        addToArray(i, var->results, f1, f2);
        printf("in analysis method printing after add to array\n");
   
    }
}

int main(int argc, char **argv){
    if(argc < 2){ 
        perror("Number of argument error");
        return EXIT_FAILURE;
    }

    struct Queue fq;
    struct Queue dq;
    struct filenode* fl =  malloc(sizeof(struct filenode));
    init(fl);
    int file_count = 0;
    fl[0].total_files = &file_count;
    struct variables *data;
    
    pthread_t *tids;
    pthread_mutex_t file_lock;
    printf("should make it here\n");
	

    // traverse optional arguments
    for(int m = 1; m < argc; m++){ 
        int length = strlen(argv[m])+1;
        char* input = (char*) malloc(length * sizeof(char));
        strcpy(input, argv[m]);
        if(input[0] == '-'){ // optional argument
            int error = op_args(input);
            if(error == 1){
                free(input);
                perror("Incorrect Optional Argument");
                exit(0);
            }
        }
        free(input);
    }
    printf("these are values of for dthread:%d for fthread:%d for a_thread:%d suffix:%s\n", d_thread, f_thread, a_thread, suf);

    int total_threads = f_thread + d_thread + a_thread;
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
        data[p].filelist = &fl[0];
        data[p].lock = &file_lock;
	    data[p].active = &active;
        error = pthread_create(&tids[p], NULL, file_traverse, &data[p]);
        //printf("thread id %ld\n", tids[p]);
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
    for(; p < total_threads - a_thread; p++){
	    data[p].filequ = &fq;
	    data[p].dirqu = &dq;
        data[p].thread_id = p;
        data[p].filelist = &fl[0];
	    data[p].active = &active;
        error = pthread_create(&tids[p], NULL, directory_traverse, &data[p]);
        //printf("thread id %ld\n", tids[p]);
        if(error != 0){
            perror("pthread_create");
            abort();
        }
    }
    printf("main is continue\n");

    int w;
    for(w = 0; w < (total_threads - a_thread); w++){
        pthread_join(tids[w], NULL);
    }

    // TODO: start analysis threads phase 2 [JSD]
    //data[0].filelist->total_files = fl->total_files;
    int numfiles = *data[0].filelist->total_files;
    //printf("the value of numfiles: %d\n", numfiles);
    //printf("error here\n");
    int comparisons = (numfiles*(numfiles-1))/2;
    //printf("the value of comparisons: %d\n", comparisons);
    int perthread = 1; //comparisons/numfiles;
    //printf("the value of perthreads: %d\n", perthread);
    struct comp_result* results = malloc(comparisons*sizeof(struct comp_result));
    //printf("here the total files is %d\n", *fl[0].total_files); 

    filenode** head = &fl;
	filenode *ptr1; 
    filenode *ptr2;
    ptr1 = fl; //data[p].filelist;
    ptr2 = fl->next; // data[p].filelist->next;
    printf("Pointer to head is %s\n", (*head)->filename);
    printf("before the while loop in main\n");
    int i = 0;
    printf("[0]Contents in fl Name of file is %s\n", fl->filename);
    printf("[0]Contents in fl Name of file is %s\n", fl->next->filename);
    printf("Name of file is %s\n", ptr1->filename);
    printf("Name of file is second file %s\n", ptr2->filename);
   while(ptr2 != NULL){
    printf("start loop\n");
        while(ptr2 != NULL){
            results[i].file1 = ptr1->filename;
            results[i].file2 = ptr2->filename;
            printf("looping\n");
            i++;
            ptr2 = ptr2->next;
        }
    ptr1 = ptr1->next;
    ptr2 = ptr1->next;
    }
   // fl = head;
   printf("FINISHED LOOP\n");
    printf("Pointer to head is %s\n", (*head)->filename);

    printf("[0]Contents in fl Name of file is %s\n", fl->filename);
    printf("[0]Contents in fl Name of file is %s\n", fl->next->filename);
    printf("Name of file is %s\n", ptr1->filename);
    if(ptr2 == NULL){
        printf("PTR 2 POINTS TO NOTHING\n");
    }
    
    



     printf("RUNNIG THREADS \n");
    int err4;
    data[p].start = 0;
    data[p].end = perthread;
    for(; p < total_threads; p ++)
    {
        data[p].thread_id = p;
        data[p].filelist = &fl[0];
        data[p].results = results;
        data[p].lock = &file_lock;
        err4 = pthread_create(&tids[p], NULL, analysis, &data[p]);
        if(err4 != 0)
        {
            perror("pthread_create");
            abort();
        }
        data[p+1].start = data[p].start + perthread;
        data[p+1].end += data[p].end + perthread;
    }
	
    for(; w < total_threads; w++){
        pthread_join(tids[w], NULL);
    }
    printf("threads are done\n");
	
        printf("%s\n",results[0].file1);
    

/*

    //data[0].filelist = data[0].filelist->next;
    //printf("[0]Name of file is %s this is the total files %d\n", data[0].filelist->filename, *data[0].filelist->total_files);
    //printf("Name of file is %d\n", data[0].filelist->totalnodes);
    //printf("[1]Name of file is %s this is the total files %d\n", data[0].filelist->next->filename, *data[0].filelist->next->total_files);
    //printf("[1]Name of file is %s\n", data[0].filelist->next->next->filename);
    //printf("Name of file is %s\n", data[1].filelist->filename);
    //printLinkedlist(data[0].filelist->head);
    freeFileNodes(data[0].filelist);
    free(results);
    destroy_lock(data->dirqu);
    destroy_lock(data->filequ);
    free(data);
    free(tids);

    return EXIT_SUCCESS;
    */
}
