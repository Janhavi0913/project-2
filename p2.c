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
    printf("[%d] Analysis Thread\n",var->thread_id);
    int i;
    for(i = var->start; i < var->end; i ++)
    {
        char *file1, *file2;
	    file1 = var->results[var->thread_id].file1;
	    file2 = var->results[var->thread_id].file2;
        filenode* f1, *f2;
        filenode* ptr = var->filelist.head;
        while(ptr != NULL)
        {
            if(strcmp(ptr->filename, file1) == 0)
                f1 = ptr;
            else if(strcmp(ptr->filename, file2) == 0)
                f2 = ptr;
            if(f1 != NULL && f2 != NULL)
                break;
            ptr = ptr->next;
        }
        addToArray(var->thread_id, var->results, f1, f2);
        free(file1);
        free(file2);
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
	int numfiles = data[0].fl->total_files[0];
    int comparisons = numfiles*(numfiles-1)/2;
    int perthread = comparisons/numfiles;
    struct comp_result* results = comparisons*malloc(sizeof(struct comp_result));
    printf("here the total files is %d\n",*fl[0].total_files); 

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
    for(p; p < total_threads - a_thread; p++){
	    data[p].filequ = &fq;
	    data[p].dirqu = &dq;
        data[p].thread_id = p;
        data[p].filelist = &fl[0];
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

	
	filelist *ptr1, *ptr2;
    ptr1 = data[p].filelist;
    ptr2 = data[p].filelist->next;
    int i = 0;
    while(ptr1 != NULL)
    {
        while(ptr2 != NULL)
        {
            results[i]->file1 = ptr1->filename;
            results[i]->file2 = ptr2->filename;
            i ++;
            ptr2 = ptr2->next;
        }
        ptr1 = ptr1->next;
        ptr2 = ptr1->next;
    }

    int err4;
    data[p].start = 0;
    data[p].end = perthreads;
    for(; p < totalthreads; p ++)
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
        data[p+1].start = data[p].start + perthreads;
        data[p+1].end += data[p].end + perthreads;
    }
	
    for(int m = 0; m < total_threads; m++){
        pthread_join(tids[m], NULL);
    }
    printf("threads are done\n");
	
	for (i = 0; i < comparisons; ++i) {
        printf("%d %s %s\n", results[i].JSD, results[i].file1, results[i].file2);
    }

    //data[0].filelist = data[0].filelist->next;
    //printf("[0]Name of file is %s this is the total files %d\n", data[0].filelist->filename, *data[0].filelist->total_files);
    //printf("Name of file is %d\n", data[0].filelist->totalnodes);
    //printf("[1]Name of file is %s this is the total files %d\n", data[0].filelist->next->filename, *data[0].filelist->next->total_files);
    //printf("[1]Name of file is %s\n", data[0].filelist->next->next->filename);
    //printf("Name of file is %s\n", data[1].filelist->filename);
    printLinkedlist(data[0].filelist->head);
    freeFileNodes(data[0].filelist);
    free(results);
    destroy_lock(data->dirqu);
    destroy_lock(data->filequ);
    free(data);
    free(tids);

    return EXIT_SUCCESS;
}
