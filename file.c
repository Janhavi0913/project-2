#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include "strbuf.c"
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

//1. create combined file 2. do computation 3. destroy combined file 4. return computation
typedef struct wordnode{
	char* word;
	int numoccur;
	int totalnodes;
	double WFD;
	struct wordnode* next;
} wordnode;

typedef struct filenode{
	char* filename;
	wordnode* head;
	int totalnodes;
	struct filenode* next;
} filenode;

wordnode* createNode(char* word){
        struct wordnode* newnode = (struct wordnode*)malloc(sizeof(struct wordnode));
        newnode->word = word;
        newnode->numoccur = 1;
        newnode->totalnodes = 1;
	newnode->WFD = 0;
        newnode->next = NULL;
        return newnode;
}

filenode* createFileNode(char* filename, wordnode* head, int totalnodes){ 
	struct filenode* thenode = (struct filenode*)malloc(sizeof(struct filenode));
	thenode->filename = filename; // dequeue from the file queue
	thenode->head = head; // call method that makes the wordnode for that file
	thenode->totalnodes = totalnodes;
	return thenode;
}

wordnode* insert(wordnode* head, char* word){
	if(head == NULL){
		wordnode* thenode  = createNode(word);
		head = thenode;
		return head;
	}
	else if(head->next == NULL){
		if(strcmp(head->word, word) > 0){
			wordnode* thenode = createNode(word);
			thenode->next = head;
			thenode->totalnodes += head->totalnodes;
			return thenode;
		}
		else if(strcmp(head->word, word) == 0){
			head->numoccur += 1;
			head->totalnodes += 1;
			return head;
		}
		else{
			wordnode* thenode = createNode(word);
			head->next = thenode;
			head->totalnodes += 1;
			return head;
		}
	}
	else{
		wordnode* ptr;
		ptr = head;
		while(ptr != NULL){
			if(strcmp(ptr->word, word) == 0){
				ptr->numoccur += 1;
				head->totalnodes += 1;
				return head;
			}
			else if(strcmp(ptr->word, word) < 0){
				if(ptr->next == NULL)
					break;
				else if(strcmp(ptr->next->word, word) > 0)
					break;
				else
					ptr = ptr->next;
			}
			else{
				wordnode* thenode = createNode(word);
				if(ptr == head){
					thenode->next = head;
					thenode->totalnodes += head->totalnodes;
					return thenode;
				}
				else{
					thenode->next = ptr;
					head->totalnodes += 1;
					return head;
				}
			}
		}
		wordnode* thenode = createNode(word);
		if(ptr->next != NULL)
			thenode->next = ptr->next;
		ptr->next = thenode;
		head->totalnodes+=1;
		return head;
	}
}

strbuf_t readFile(int fd){
	char *a = malloc(sizeof(char));
	int rval = read(fd, a, sizeof(char));
	char curr;
	strbuf_t file;
	sb_init(&file, 5);
	while(rval == 1){
		if(ispunct(a[0]) == 0 || a[0] == '-' || isspace(a[0]) == 0 || a[0] == ' '){
			curr = tolower(a[0]);
			if(curr != '\n')
				sb_append(&file, curr);
			else
				sb_append(&file, ' ');
		}
		rval = read(fd, a, sizeof(char));
	}
	close(fd);
	return file;
}

void printLinkedlist(wordnode* head){
	wordnode* ptr = head;
    while(ptr != NULL){
        printf("%s: %f\n", ptr->word, ptr->WFD);
        ptr = ptr->next;
    }
}

void freeNodes(wordnode* head){
    if(head != NULL)
        return;
    freeNodes(head->next);
    free(head);
}

void* file_traverse(void *A){
	struct variables *var = A;
	char *curfile = NULL;
	int proceed = fil_dequeue(var->dirqu, var->filqu, &curfile, var->active);
    if(proceed != 0){
		pthread_exit(NULL);
    }

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

	/*  TODO ADD TO FILENODE FILELIST
		addtofileList(var->filelist, curfile, head); // this will call createfilenode
	*/

	freeNodes(head);
}

int main(int argc, char **argv){
	if(argc < 2){ // too little arguments
		printf("Number of argument error\n");
		return EXIT_FAILURE;
	}

	//starting file threads
	pthread_t *file_tid;
	int err2, active = d_thread;
    data = malloc(f_thread * sizeof(struct variables));
    file_tid = malloc(f_thread * sizeof(pthread_t));
    for(int m = 0; m < f_thread; m++){
	    data[m].filequ = &fq;
	    data[m].dirqu = &dq;
        data[m].thread_id = m;
	    data[m].active = &active;
        err2 = pthread_create(&file_tid[m], NULL, file_traverse, &data[m]);
        printf("thread id %ld\n", file_tid[m]);
        if(err2 != 0){
            perror("pthread_create");
            abort();
        }
    }

	/*char* f1 = argv[1];
    	int fd = open(f1, O_RDONLY);
    	if(fd == -1)
    		perror(argv[2]);
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
	
	filenode* f = createFileNode(f1, head, head->totalnodes);
	freeNodes(head);
	*/
	return EXIT_SUCCESS;
}
