#include <stdlib.h>
#include <math.h>
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
	double WFD;
	struct wordnode* next;
} wordnode;

typedef struct filenode
{
	char* filename;
	wordnode* head;
	int totalnodes;
	struct filenode* next;
} filenode;

wordnode* createNode(char* word){ // this would have to call insert method
        struct wordnode* newnode = (struct wordnode*)malloc(sizeof(struct wordnode));
        newnode->word = word;
        newnode->numoccur = 1;
        newnode->totalnodes = 1;
	newnode->WFD = 0;
        newnode->next = NULL;
        return newnode;
}

void freeNodes(wordnode* head)
{
    if(head != NULL)
        return;
    freeNodes(head->next);
    free(head);
}

void printLinkedlist(wordnode* head)
{
	wordnode* ptr = head;
    while(ptr != NULL)
    {
        printf("%s\n", ptr->word);
        ptr = ptr->next;
    }
}

double totalcomputation(wordnode* file1, wordnode* file2, wordnode* file)
{
	double KLD1 = 0, KLD2 = 0, JSD = 0;
	wordnode* ptr = file;
	while(ptr != NULL)
	{
		while(strcmp(file1->word, ptr->word) != 0)
			ptr = ptr->next;
		KLD1 += (file1->numoccur*log2((file1->numoccur)/(ptr->numoccur)));
		ptr = ptr->next;
		file1 = file1->next;
	}
	wordnode* ptr2 = file;
	while(ptr2 != NULL)
	{
		while(strcmp(file2->word, ptr2->word) != 0)
			ptr2 = ptr2->next;
		KLD2 += (file2->numoccur*log2((file2->numoccur)/(ptr2->numoccur)));
		ptr2 = ptr2->next;
		file2 = file2->next;
	}
	JSD = sqrt((0.5*KLD1)+(0.5*KLD2));
	return JSD;
}

double createCombined(filenode* f1, filenode* f2)
{
	wordnode* fhead;
	wordnode* ptr1;
	wordnode* ptr2;
	wordnode* fptr;
	wordnode* prev;
	ptr1 = f1->head->next;
	ptr2 = f2->head;
	fhead = createNode(f1->head->word);
	fhead->numoccur = f1->head->numoccur;
	fhead->totalnodes = f1->totalnodes + f2->totalnodes;
	fptr = fhead;
	while(ptr1 != NULL)
	{
		fptr->next = createNode(ptr1->word);
		fptr->next->numoccur = ptr1->numoccur;
		fptr = fptr->next;
		ptr1 = ptr1->next;
	}
	fptr->next = NULL;
	fptr = fhead;
	while(ptr2 != NULL)
	{
		if(strcmp(fptr->word, ptr2->word) == 0)
		{
			fptr->numoccur += ptr2->numoccur;
			ptr2 = ptr2->next;
			if(fptr->next != NULL)
			{	
				prev = fptr;
				fptr = fptr->next;
			}
			else
			{
				break;
			}
		}
		else if(strcmp(fptr->word, ptr2->word) < 0)
		{
			if(fptr->next != NULL)
			{
				prev = fptr;
				fptr = fptr->next;
			}
			else
				break;
		}
		else
		{
			wordnode* thenode = createNode(ptr2->word);
			thenode->numoccur = ptr2->numoccur;
			thenode->next = fptr;
			if(fptr == fhead)
			{
				thenode->totalnodes = fhead->totalnodes;
				fhead = thenode;
			}
			else
				prev->next = thenode;
			fptr = thenode;
			ptr2 = ptr2->next;
		}
	}
	if(ptr2 != NULL)
	{
		while(ptr2 != NULL)
		{
			fptr->next = ptr2;
			fptr = fptr->next;
			ptr2 = ptr2->next;
		}
	}
	fptr = fhead;
	while(fptr != NULL)
	{
		fptr->WFD = (double)fptr->numoccur/(double)fhead->totalnodes;
		fptr = fptr->next;
	}

	double result = totalcomputation(f1->head, f2->head, fhead);
	freeNodes(fhead);
	return result;
}
