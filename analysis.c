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
	double totalnodes;
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

filenode* createFileNode(char* filename, wordnode* head, int totalnodes) // make this a void* return, and change to accept struct as argument
{
	struct filenode* thenode = (struct filenode*)malloc(sizeof(struct filenode));
	thenode->filename = filename; // dequeue from the file queue
	thenode->head = head; // call method that makes the wordnode for that file
	thenode->totalnodes = totalnodes;
	return thenode;
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

wordnode* createCombined(wordnode* file1, wordnode* file2)
{
	wordnode* fhead;
	wordnode* ptr1;
	wordnode* ptr2;
	wordnode* fptr;
	wordnode* prev;
	ptr1 = file1->next;
	ptr2 = file2;
	fhead = createNode(file1->word);
	fhead->numoccur = file1->numoccur;
	fhead->totalnodes = file1->totalnodes + file2->totalnodes;
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
	return fhead;
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

int main(int argc, char **argv)
{
	wordnode* f1 = createNode("hi");
	wordnode* f11 = createNode("there");
	f1->next = f11;
	f1->numoccur = 2;
	f1->totalnodes = 4;
	f11->numoccur = 2;
	//filenode* file1 = createFileNode("f1.txt", f1, f1->totalnodes);

	wordnode* f2 = createNode("apple");
	wordnode* f21 = createNode("hi");
	wordnode* f22 = createNode("out");
	wordnode* f23 = createNode("there");
	wordnode* f24 = createNode("z");
	f2->next = f21;
	f21->next = f22;
	f22->next = f23;
	f23->next = f24;
	f2->numoccur = 1;
	f2->totalnodes = 6;
	f21->numoccur = 2;
	f22->numoccur = 1;
	f23->numoccur = 1;
	f24->numoccur = 1;
	//filenode* file2 = createFileNode("f2.txt", f2, f2->totalnodes);

	wordnode* result = createCombined(f2, f1);
	printLinkedlist(result);

	freeNodes(f1);
	freeNodes(f2);
	freeNodes(result);

	/*wordnode* f = createNode("hi");
	wordnode* f01 = createNode("out");
	wordnode* f02 = createNode("there");
	f->next = f01;
	f->numoccur = 0.5;
	f01->next = f02;
	f01->numoccur = 0.125;
	f02->numoccur = 0.375;

	double JSD;
	JSD = totalcomputation(f1, f2, f);
	printf("Result: %f\n", JSD);*/
	return EXIT_SUCCESS;
}
