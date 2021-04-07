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

typedef struct wordnode
{
	char* word;
	int numoccur;
	int totalnodes;
	struct wordnode* next;
} wordnode;

wordnode* createNode(char* word)
{
        struct wordnode* newnode = (struct wordnode*)malloc(sizeof(struct wordnode));
        newnode->word = word;
        newnode->numoccur = 1;
        newnode->totalnodes = 1;
        newnode->next = NULL;
        return newnode;
}

wordnode* insert(wordnode* head, char* word)
{
	printf("In insert: the word we're inputting is %s\t", word);
	if(head == NULL)
	{
		wordnode* thenode  = createNode(word);
		head = thenode;
		head->totalnodes+=1;
		printf("Head1: %s\n", head->word);
		return head;
	}
	else
	{
		wordnode* ptr;
		ptr = head;
		printf("Head: %s\n", head->word);
		while(ptr->next != NULL)
		{
			if(strcmp(ptr->word, word) == 0)
			{
				printf("Current word: %s and word in list: %s\n", word, ptr->word);
				ptr->numoccur += 1;
				printf("Word occured more than once: %s\n", head->word);
				return head;
			}
			else if((strcmp(ptr->word, word) > 0 && strcmp(ptr->next->word, word) < 0) || (strcmp(ptr->word, word) > 0 && ptr->next == NULL))
			{
				printf("Pointer is %s and the word is %s", ptr->word, word);
				break;
			}
			else if(strcmp(ptr->word, word) < 0)
			{
				printf("Pointer is %s and the word is %s", ptr->word, word);
				wordnode* thenode = createNode(word);
				thenode->next = head;
				thenode->totalnodes += head->totalnodes;
				return thenode;
			}
			ptr = ptr->next;
		}
		wordnode* thenode = createNode(word);
		if(ptr->next != NULL)
			thenode->next = ptr->next;
		ptr->next = thenode;
		head->totalnodes+=1;
		return head;
	}
}

/*wordnode* sortLexicographically()
{
	int i;
	int j;
	for(i=0;i<head->totalnodes - 1;i++)
	{
		for(j=i+1;j<10;j++)
		{
			if(strcmp(str[i],str[j])>0)
			{
				strcpy(temp,str[i]);
				strcpy(str[i],str[j]);
				strcpy(str[j],temp);
			}
		}
	}
}*/

void freeNodes(wordnode* head)
{
    if(head != NULL)
        return;
    freeNodes(head->next);
    free(head);
}

wordnode* createLinkedList(wordnode* head, int fd)
{
	char *a = malloc(sizeof(char));
	int rval = read(fd, a, sizeof(char));
	char curr;
	strbuf_t word;
	sb_init(&word, 5);
	int i;
	while(rval == 1)
	{
		while(isspace(a[0]) == 0 && rval == 1)
		{
			curr = tolower(a[0]);
			if(ispunct(curr) == 0 || curr == '-')
			{
            	sb_append(&word, curr);
            }
            rval = read(fd, a, sizeof(char));
        }
        printf("Word before insert: %s\n", word.data);
        if(head != NULL)
        	printf("Head before insert: %s\n", head->word);
        head = insert(head, word.data);
        printf("Head after insert: %s\n", head->word);
        for(i = 0; i < word.length; i ++)
        	remove(word.data);
        printf("Head after insert: %s\n", head->word);
		while(isspace(a[0]) != 0 && rval == 1)
		{
			rval = read(fd, a, sizeof(char));
		}
	}
	return head;
}

void printLinkedlist(wordnode* head)
{
	wordnode* ptr = head;
	printf("Head at the end: %s\n", ptr->word);
    while(ptr != NULL)
    {
        printf("%s\n", ptr->word);
        ptr = ptr->next;
    }
    printf("\n");
}

int main(int argc, char **argv)
{
    if(argc < 2)
    { // too many arguments or too little arguments
    	printf("Number of argument error\n");
    	return EXIT_FAILURE;
    }
    char* f1 = argv[1];
    int fd = open(f1, O_RDONLY);
    if(fd == -1)
    	perror(argv[2]);
    wordnode* start;
    start = createLinkedList(NULL, fd);
    printf("Words in lexicographic order:\n");
    printLinkedlist(start);
    printf("Number of words in file: %d\n", start->totalnodes);
}