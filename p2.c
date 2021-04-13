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
//#include "file.c"

struct variables{
    int d_threads, f_threads, a_threads;
    char* suffix;
    struct Queue* filequ;
    struct Queue* dirqu;
    struct filenode* filelist;
    int active;
}variables;

int op_args (char* input, struct variables *var){
    if((input[1] != 'd') || (input[1] != 'f') || (input[1] != 'a') || (input[1] != 's') ){
            return 1;
    }
    if(isdigit(input[2])){
        int length = strlen(input) - 2;
        char* temp1 = (char*) malloc(length * sizeof(char));
        strcpy(temp1,input[2]);
        for(int p = 3; p < length; p++){
            strcat(temp1,input[p]);
        }
        if(atoi(temp1) <= 0){
            return 1;
        }
        if(input[1] == 'd'){
            var->d_threads = atoi(temp1);
        } 
        if(input[1] == 'f'){
            var->f_threads = atoi(temp1);
        }
        if(input[1] == 'a'){
            var->a_threads = atoi(temp1);
        }
        free(temp1);
        return 0;
    }
    if(input[1] == 's'){
        int length = strlen(input) - 2;
        char* temp2 = (char*) malloc(length * sizeof(char));
        strcpy(temp2,input[2]);
        for(int p = 2; p < length; p++){
            strcat(temp2,input[p]);
        }
        strcpy(var.suffix, temp2);
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
    char* curdir;
    dir_dequeue(var.dirqu, var.filequ, &curdir, &var.active);
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
                    fil_enqueue(var.filequ, pathname);
                    continue;
                }
            }  
            else{ // this is a directory add to directory queue
                dir_enqueue(var.dirqu, pathname);
            }
        free(pathname);
        }
    }
    free(curdir);
    if(isEmpty(var.dirqu) == 0){
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
    data.filequ = createQueue(1000);
    data.dirqu = createQueue(NULL);
    data.d_threads = 1, data.f_threads = 1, data.a_threads = 1, data.inactive = 0;
    data.suffix = ".txt";
    //data.filelist = createNode(NULL);
    
    for(int m = 1; m < argc; m++){ // traverse through arguments look for optional arguments
        int length = strlen(argv[m]);
        char* input = (char*) malloc(length * sizeof(char));
        strcpy(input, argv[m]);
        if(input[0] == '-'){ // this is an optional argument
            int error = op_args(input, &data);
            if(error == 1){
                perror("Incorrect Optional Argument");
                return EXIT_FAILURE;
            }
        }
        free(input);
    }

    /* int err1;
    pthread_t fil_tid[data.f_threads]; // starting file threads
    for(int m = 0; m < data.f_threads; m++){
        err1 = pthread_create(&fil_tid[m], NULL, WFD, &data);
        if(err1 != 0){
            perror("pthread_create");
            abort();
        }
    } */

    for(int m = 1; m < argc; m++){ // traverse through arguments get directory and files
        int length = strlen(argv[m]);
        char* input = (char*) malloc(length * sizeof(char));
        strcpy(input, argv[m]);
        if(input[0] == '-'){ // this is an optional argument - ignore
            continue;
        }
        else{
            if(isdir(input) == 1){ // this is a file add to file queue
                fil_enqueue(data.filequ, input);
            }
            else{ // this is a directory add to directory queue
                dir_enqueue(data.dirqu, input);
            }
        free(input);
        }
    }

    int err2;
    pthread_t dir_tid[data.d_threads]; // starting directory threads
    for(int m = 0; m < data.d_threads; m++){
        err2 = pthread_create(&dir_tid[m], NULL, directory_traverse, &data);
        if(err2 != 0){
            perror("pthread_create");
            abort();
        }
    }

    for(int m = 0; m < data.d_threads; m++){
        pthread_join(dir_tid[m], NULL);
    }
    close_queue(data.filequ);
    
    for(int m = 0; m < data.f_threads; m++){
        pthread_join(fil_tid[m], NULL);
    }

    // TODO: start analysis threads and phase 2 [JSD]
    
    return EXIT_SUCCESS;
}

/*
while(isEmpty(fdata.filequ) == 0){
    char* arg;
    dequeue(data.filequ, &arg);
    printf("File name: %s\n",arg);
}
printf("emptied list 1\n");
while(isEmpty(data.dirqu) == 0){
    char* arg;
    dequeue(data.dirqu, &arg);
    printf("Directory name: %s\n",arg);;
}
printf("emptied list 2\n");
*/
