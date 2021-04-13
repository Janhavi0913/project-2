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
	char* filename;
	int numoccur;
	int totalnodes;
	double WFD;
	struct wordnode* next;
} wordnode;

wordnode* createNode(char* word)
{
        struct wordnode* newnode = (struct wordnode*)malloc(sizeof(struct wordnode));
        newnode->word = word;
	newnode->filename = NULL;
        newnode->numoccur = 1;
        newnode->totalnodes = 1;
	newnode->WFD = 0;
        newnode->next = NULL;
        return newnode;
}

wordnode* insert(wordnode* head, char* word, char* filename)
{
	if(head == NULL)
	{
		wordnode* thenode  = createNode(word);
		head = thenode;
		head->filename = filename;
		return head;
	}
	else if(head->next == NULL)
	{
		if(strcmp(head->word, word) > 0)
		{
			wordnode* thenode = createNode(word);
			thenode->next = head;
			thenode->totalnodes += head->totalnodes;
			thenode->filename = filename;
			head->filename = NULL;
			return thenode;
		}
		else if(strcmp(head->word, word) == 0)
		{
			head->numoccur += 1;
			head->totalnodes += 1;
			return head;
		}
		else
		{
			wordnode* thenode = createNode(word);
			head->next = thenode;
			head->totalnodes += 1;
			return head;
		}
	}
	else
	{
		wordnode* ptr;
		ptr = head;
		while(ptr != NULL)
		{
			if(strcmp(ptr->word, word) == 0)
			{
				ptr->numoccur += 1;
				head->totalnodes += 1;
				return head;
			}
			else if(strcmp(ptr->word, word) < 0)
			{
				if(ptr->next == NULL)
					break;
				else if(strcmp(ptr->next->word, word) > 0)
					break;
				else
					ptr = ptr->next;
			}
			else
			{
				wordnode* thenode = createNode(word);
				if(ptr == head)
				{
					thenode->next = head;
					thenode->totalnodes += head->totalnodes;
					thenode->filename = filename;
					head->filename = NULL;
					return thenode;
				}
				else
				{
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

strbuf_t readFile(int fd)
{
	char *a = malloc(sizeof(char));
	int rval = read(fd, a, sizeof(char));
	char curr;
	strbuf_t file;
	sb_init(&file, 5);
	while(rval == 1)
	{
		if(ispunct(a[0]) == 0 || a[0] == '-' || isspace(a[0]) == 0 || a[0] == ' ')
		{
			curr = tolower(a[0]);
			if(curr != '\n')
				sb_append(&file, curr);
		}
		rval = read(fd, a, sizeof(char));
	}
	close(fd);
	return file;
}

void printLinkedlist(wordnode* head)
{
	wordnode* ptr = head;
    while(ptr != NULL)
    {
        printf("%s: %f\n", ptr->word, ptr->WFD);
        ptr = ptr->next;
    }
}

void freeNodes(wordnode* head)
{
    if(head != NULL)
        return;
    freeNodes(head->next);
    free(head);
}

int main(int argc, char **argv)
{
	if(argc < 2)
	{ 
		// too little arguments
		printf("Number of argument error\n");
		return EXIT_FAILURE;
	}
	char* f1 = argv[1];
    	int fd = open(f1, O_RDONLY);
    	if(fd == -1)
    		perror(argv[2]);
	strbuf_t file = readFile(fd);
	int i = 0;
	char delim[2] = " ";
	char* str = strtok(file.data, delim);
	wordnode* head = insert(NULL, str, f1);
	i += strlen(str);
	while(i < file.length)
	{
		str = strtok(NULL, delim);
		if(str == NULL)
			break;
		head = insert(head, str, f1);
		i += strlen(str);
	}
	sb_destroy(&file);
	wordnode* ptr = head;
	while(ptr != NULL)
	{
		ptr->WFD = ptr->numoccur/head->totalnodes;
		ptr = ptr->next;
	}
	freeNodes(head);
	return EXIT_SUCCESS;
}
