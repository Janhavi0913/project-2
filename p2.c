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

int num_threads[3] = {1,1,1};
char* suf = ".txt"; // file name suffix
struct Queue* filequ;
struct Queue* dirqu;

int op_args (char* arg){
    if((arg[1] != 'd') || (arg[1] != 'f') || (arg[1] != 'a') || (arg[1] != 's') ){
            return 1;
    }
    if(isdigit(arg[2])){
        char* temp1;
        for(int p = 2; p < strlen(arg); p++){
            temp1 += arg[p];
        }
        if(atoi(temp1) == 0){
            return 1;
        }
        if(arg[1] == 'd'){
            num_threads[0] = atoi(temp1);
        } 
        if(arg[1] == 'f'){
            num_threads[1] = atoi(temp1);
        }
        if(arg[1] == 'a'){
            num_threads[2] = atoi(temp1);
        }
        return 0;
    }
    else if(arg[1] == 's'){
        char* temp2;
        for(int p = 2; p < strlen(arg); p++){
            temp2 += arg[p];
        }
        strcpy(suf, temp2);
    }
    else
        return 1; 
}
int isdir(char *name) {
    struct stat data;
	int err = stat(name, &data);
	if (err) {// should confirm err == 0
        printf("should not be here\n");
		perror(name);  // print error message
		return 0;
	}
	if (S_ISDIR(data.st_mode)) { // S_ISDIR macro is true if the st_mode says the file is a directory
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
//ToDo: pass in struct that contains fileq and dirq
void* directory_traverse(void* arg){
    char* curdir = dequeue(dirqu);
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
            if(isdir(pathname) == 1){ // this is a file add to file queue
                if(check_suffix(pathname) == 1){
                    enqueue(filequ, pathname);
                    continue;
                }
            }  
            else{// this is a directory add to directory queue
                enqueue(dirqu, pathname);
            }
        free(pathname);
        }
    }
    if(isEmpty(dirqu) == 1){
        directory_traverse(dirqu);
    }
    //pthread_exit(NULL);
}
int main(int argc, char **argv){
    if(argc < 2){ // incorrect arguments
        perror("Number of argument error\n");
        return EXIT_FAILURE;
    }
    filequ = createQueue();
    dirqu = createQueue();
    for(int m = 1; m < argc; m++){ // traverse through arguments
        int length = strlen(argv[m]);
        char* arg = (char*) malloc(length * sizeof(char));
        strcpy(arg, argv[m]);
        if(arg[0] == '-'){ // this is an optional argument
            int error = op_args(arg);
            if(error == 1){
                perror("optional argument");
                return EXIT_FAILURE;
            }
        }
        else if(isdir(arg) == 1){ // this is a file add to file queue
            enqueue(filequ, arg);
        }
        else{// this is a directory add to directory queue
            enqueue(dirqu, arg);
        }
        free(arg);
    }
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
    /* int err;
    pthread_t tid[num_threads[0]];
    for(int m = 0; m < num_threads[0]; m++){
        err = pthread_create(&tid[m], NULL, directory_traverse, NULL);
        if(err != 0){
            perror("pthread_create");
            abort();
        }
    }
    for(int m = 0; m < num_threads[0]; m++){
        pthread_join(tid[m], NULL);
    }  */

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
}fileWFD;    printf("added now attempting to empty list\n");
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
*/