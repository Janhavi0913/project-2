#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>
#include <unistd.h>
//#include "strbuf.c"
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

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
	int* total_files;
	struct filenode* next;
} filenode;

int init(struct filenode *f){
	f->filename = NULL;
	f->head = NULL;
	f->totalnodes = 0;
	f->next = NULL;
    
    return 0;
}

wordnode* createNode(char* word){
        struct wordnode* newnode = (struct wordnode*)malloc(sizeof(struct wordnode));
		char *word_insert = (char*) malloc((strlen(word)+1) * sizeof(char));
		strcpy(word_insert, word);
        newnode->word = word_insert;
        newnode->numoccur = 1;
        newnode->totalnodes = 1;
		newnode->WFD = 0;
        newnode->next = NULL;
        return newnode;
}

wordnode* insert(wordnode* head, char* word){
	//printf("made it inside the insert. the word is %s\n", word);
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
		curr = tolower(a[0]);
		if(ispunct(a[0]) == 0 && isspace(a[0]) == 0){
			sb_append(&file, curr);
		}
		else if(a[0] == '-' && a[0] == ' '){
			sb_append(&file, curr);
		}
		else{
			sb_append(&file, ' ');
		}
		//printf("the letter is %c\n", a[0]);
		rval = read(fd, a, sizeof(char));
	}
	//printf("this is what is in file %s\n",file.data);
	close(fd);
	free(a);
	return file;
}

int addToFileList(filenode *fl, char* fname, wordnode *wl, int id, pthread_mutex_t *l){
    //printf("[%d] FILE Thread waiting for lock in add \n",id);
	pthread_mutex_lock(l); // only one thread add to list at a time
	//("[%d]Grabbed the lock\n", id);

    struct filenode* add = (struct filenode*)malloc(sizeof(struct filenode));
	add->filename = fname;
	add->head = wl;
	add->totalnodes = wl->totalnodes;
	//add->total_files = fl->total_files + 1;
	add->next = NULL;
	add->total_files = fl->total_files;
	//printf("[%d] has made a new node \n",id);
	
	//printf("this is what is fl filename %d\n", *fl->total_files);
	
	if(*fl->total_files == 0){
		//printf("[%d] adding to front\n", id);
		*fl = *add;
		++(*fl->total_files);
		
		//printf("[%d] Has added to front %d\n",id, *fl->total_files);
	}
	else{
		//printf("[%d] total files was not 0 adding to end %d\n",id, *fl->total_files);
		filenode *ptr = fl;
		filenode *prev;
		while(ptr != NULL){
			prev = ptr;
			ptr = ptr->next;
		}
		prev->next = add;
		++(*fl->total_files);
		//printf("[%d] total files is now %d\n",id, *fl->total_files);
	}

    //printf("[%d] FT Name of the file is %s\n", id,fl->filename);

    //printf("[%d] FILE Thread is going to released the lock in add \n",id);
	pthread_mutex_unlock(l); // now we're done
    //printf("[%d] FILE Thread has released the lock in add \n",id);
	return 0;
}

void printLinkedlist(wordnode* head){
	wordnode* ptr = head;
    while(ptr != NULL){
        printf("%s: %f\n", ptr->word, ptr->WFD);
        ptr = ptr->next;
    }
}

void freeWordNodes(wordnode* head){

	wordnode* tmp;
	while(head != NULL){
		tmp = head;
		head = head->next;
		free(tmp->word);
		//free(tmp->numoccur);
		free(tmp);
	}
}
void freeFileNodes(filenode* head){
	filenode* tmp;
	while(head != NULL){
		tmp = head;
		head = head->next;
		freeWordNodes(tmp->head);
		free(tmp->filename);
		free(tmp);
	}
}
