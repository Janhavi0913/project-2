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
    else if(arg[1] == 's' && arg[2] != '\0'){
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
		perror(name);  // print error message
		return 0;
	}
	if (S_ISDIR(data.st_mode)) { // S_ISDIR macro is true if the st_mode says the file is a directory
		return 0;
	} 
	return 1;
}
void* directory_traverse(void* arg){
    char* curdir = dequeue(dirqu);
    DIR *pdir = opendir(curdir);
    struct dirent *entries;
    if(pdir == NULL){
        perror("Directory cannot be open");
        abort();
    }
    while((entries = readdir(pdir)) != NULL){
        char* name;
        strcpy(name, curdir);
        strcat(name,"/");
        strcat(name,entries->d_name);
        if(isdir(name) == 1){ // this is a file add to file queue
            int find = strlen(curdir)+1; 
            for(int m = find; m < strlen(name); m++){
                if(name[m] == '.'){
                    char* filetype;
                    for(int p = m; p < strlen(name); p++){
                        strncat(filetype, &name[p],1);
                    }
                    if(strcmp(filetype,suf) == 0){
                        enqueue(filequ, name);
                    }
                    break;
                }
            }  
        }
        else{// this is a directory add to directory queue
            enqueue(dirqu, name);
        }
    }
    if(isEmpty(dirqu) == 1){
        directory_traverse(dirqu);
    }
    pthread_exit(NULL);
}
int main(int argc, char **argv){
    if(argc < 2){ // incorrect arguments
        printf("Number of argument error\n");
        return EXIT_FAILURE;
    }
    queue_init(&filequ);
    queue_init(&dirqu);
    for(int m = 1; m < argc; m++){ // traverse through arguments
        char* arg = argv[m];
        if(arg[0] == '-'){ // this is an optional argument
            int error = op_args(arg);
            if(error == 1){
                perror("optional argument");
                abort();
            }
        }
        else if(isdir(arg) == 1){ // this is a file add to file queue
            enqueue(filequ, arg);
        }
        else{// this is a directory add to directory queue
            enqueue(dirqu, arg);
        }
    }
    while(isEmpty(filequ) == 1){
        char* name = dequeue(filequ);
        printf("File name: %s\n",name);
    }
    while(isEmpty(dirqu) == 1){
        char* name = dequeue(dirqu);
        printf("File name: %s\n",name);
    }
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
    } 
 */
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
*/