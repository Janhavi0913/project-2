#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "queue.c"
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

struct variables{
    int d_threads, f_threads, a_threads;
    char* suffix;
    struct Queue* filequ;
    struct Queue* dirqu;
}variables;

int op_args (char* input, struct variables *var){
    if((input[1] != 'd') || (input[1] != 'f') || (input[1] != 'a') || (input[1] != 's') ){
            return 1;
    }
    if(isdigit(input[2])){
        char* temp1;
        for(int p = 2; p < strlen(input); p++){
            temp1 += input[p];
        }
        if(atoi(temp1) == 0){
            return 1;
        }
        if(input[1] == 'd'){
            var.d_threads = atoi(temp1);
        } 
        if(input[1] == 'f'){
            var.f_threads = atoi(temp1);
        }
        if(input[1] == 'a'){
            var.a_threads = atoi(temp1);
        }
        return 0;
    }
    else if(input[1] == 's'){
        char* temp2;
        for(int p = 2; p < strlen(input); p++){
            temp2 += input[p];
        }
        strcpy(var.suffix, temp2);
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

int check_suffix(char* filename, struct variables *var){
    int fnl = strlen(filename);
    int sufl = strlen(var.suffix);
    int dif = fnl - sufl;
    if(fnl >= sufl){
        if(strcmp(filename + dif, var.suffix) == 0){
            return 1;
        }
    }
    return 0;
}

void* directory_traverse(struct variables *var){
    char* curdir = dequeue(var.dirqu);
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
            int length = strlen(curdir) + strlen(fname) + 1;
            char* pathname = (char*) malloc(length * sizeof(char));
            strcpy(pathname,curdir);
            strcat(pathname,"/");
            strcat(pathname,entries->d_name);
            if(isdir(pathname) == 1){ // this is a file 
                if(check_suffix(pathname,var) == 1){ // add to file queue if the suffix match
                    enqueue(var.filequ, pathname);
                    continue;
                }
            }  
            else{ // this is a directory add to directory queue
                enqueue(var.dirqu, pathname);
            }
        free(pathname);
        }
    }
    if(isEmpty(var.dirqu) == 1){
        directory_traverse(var);
    }
    pthread_exit(NULL);
}

int main(int argc, char **argv){
    if(argc < 2){ 
        perror("Number of argument error\n");
        return EXIT_FAILURE;
    }
    struct variables *data = malloc(sizeof(*data));
    data.filequ = createQueue();
    data.dirqu = createQueue();
    data.d_threads = 1, data.f_threads = 1, data.a_threads = 1;
    data.suffix = ".txt";

    for(int m = 1; m < argc; m++){ // traverse through arguments
        int length = strlen(argv[m]);
        char* input = (char*) malloc(length * sizeof(char));
        strcpy(input, argv[m]);
        if(input[0] == '-'){ // this is an optional argument
            int error = op_args(input, &data);
            if(error == 1){
                perror("optional argument");
                return EXIT_FAILURE;
            }
        }
        else if(isdir(input) == 1){ // this is a file add to file queue
            enqueue(data.filequ, input);
        }
        else{ // this is a directory add to directory queue
            enqueue(data.dirqu, input);
        }
        free(input);
    }
    
    int err;
    pthread_t tid[data.d_threads];
    for(int m = 0; m < data.d_threads; m++){
        err = pthread_create(&tid[m], NULL, directory_traverse, &data);
        if(err != 0){
            perror("pthread_create");
            abort();
        }
    }
    for(int m = 0; m < data.d_threads; m++){
        pthread_join(tid[m], NULL);
    }

    return 0;
}

/*
typedef struct word_LL{ // this is a linked list that holds the words in the files and their frequencies
    char* word;
    int count;
    int freq;
    struct word_LL next;
}word_LL;

typedef struct fileWFD{ // this is a linked list that holds the name, word list, and their total for the WFD of the file
    char* path;
    struct word_LL;
    int totalwords;
}fileWFD;
   
while(isEmpty(filequ) == 1){
    char* arg;
    strcpy(arg,dequeue(filequ));
    printf("File name: %s\n",arg);
}
printf("emptied list 1\n");
while(isEmpty(dirqu) == 1){
    printf("DIrectory name: %s\n",dequeue(dirqu));
}
printf("emptied list 2\n");

printf("added now attempting traverse\n");
    directory_traverse(NULL);
    printf("i finished traversing direct\n");
    while(isEmpty(filequ) == 1){
        printf("File name %s\n", dequeue(filequ));
    }
    printf("Finished with files\n");
    while(isEmpty(dirqu) == 1){
        printf("Directory name %s\n", dequeue(dirqu));
    }
    printf("Finished with Directories\n");
*/