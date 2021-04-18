#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
//#include "strbuf.c"
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

typedef struct comp_result {
    char *file1, *file2;
    unsigned totalwords;     // word count of file 1 + file 2
    double JSD;     // JSD between file 1 and file 2
}comp_result;

comp_result* Create(char* f1, char* f2, unsigned total, double JSD)
{
	struct comp_result* res = (struct comp_result*)malloc(sizeof(struct comp_result));
	res->file1 = f1;
	res->file2 = f2;
	res->totalwords = total;
	res->JSD = JSD;
	return res;
}

double totalcomputation(wordnode* file1, wordnode* file2, wordnode* file)
{
	double KLD1 = 0, KLD2 = 0, JSD = 0;
	wordnode *pf1, *pf2;
	wordnode* ptr = file;
	while(ptr != NULL)
	{
		while(strcmp(file1->word, ptr->word) != 0)
			ptr = ptr->next;
		printf("****************************\n");
		printf("WFD of Word in File: %f Word: %s Num occur: %d Mean Word Frequency in combined file: %f\n",file1->WFD,file1->word,file1->numoccur,ptr->WFD);
		KLD1 += (file1->WFD*log2((file1->WFD)/(ptr->WFD))); // need to fix how we get mean WFD
		printf("Current KLD: %f\n", KLD1);
		ptr = ptr->next;
		file1 = file1->next;
	}
	printf("KLD1 IS %f\n",KLD1);
	printf("****************************\n");
	ptr = file;
	while(ptr != NULL)
	{
		while(strcmp(file2->word, ptr->word) != 0)
			ptr = ptr->next;
		printf("WFD of Word in File: %f Word: %s Num occur: %d Mean Word Frequency in combined file: %f\n",file2->WFD,file2->word,file2->numoccur,ptr->WFD);
		KLD2 += (file2->WFD*log2((file2->WFD)/(ptr->WFD)));
		printf("Current KLD: %f\n", KLD2);
		ptr = ptr->next;
		file2 = file2->next;
	}
	printf("KLD2 IS %f\n",KLD2);
	printf("****************************\n");
	JSD = sqrt((0.5*KLD1)+(0.5*KLD2));
	printf("TOTALCOMPUTATION RESULT IS %f\n",JSD);
	return JSD;
}

wordnode* createCombined(wordnode* f1, wordnode* f2)
{
	printf("The word count for file 1 here is %d\n",f1->totalnodes);
	printf("The word count for file 2 here is %d\n",f2->totalnodes);
	if(f1 == NULL || f2 == NULL)
	{
		if(f1 == NULL && f2 == NULL)
			return NULL;
		else{
			if(f1 == NULL)
				return f2;
			else
				return f1;
			//return sqrt(0.5);
		}
	}
	wordnode* fhead;
	wordnode* ptr1;
	wordnode* ptr2;
	wordnode* fptr;
	wordnode* prev;

	if(f2 != NULL){
		ptr2 = f2;
	}
	
	printf("CREATING NODE WITH HEAD AS %s\n",f1->word);
	fhead = createNode(f1->word);
	printf("FINISHED CREATING NODE WITH HEAD AS %s\n",fhead->word);
	fhead->numoccur = f1->numoccur;
	printf("The total words for f1 here is %d\n",f1->totalnodes);
	printf("The total words for f2 here is %d\n",f2->totalnodes);
	fhead->totalnodes = f1->totalnodes + f2->totalnodes;
	printf("COMBINED TOTAL OF WORDS IS %d\n", fhead->totalnodes);
	
	if(f1->next != NULL){
		ptr1 = f1->next;
		fptr = fhead;
		while(ptr1 != NULL)
		{
			fptr->next = createNode(ptr1->word);
			fptr->next->numoccur = ptr1->numoccur;
			fptr = fptr->next;
			ptr1 = ptr1->next;
		}
		fptr->next = NULL;
	}

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
	ptr1 = f1;
	ptr2 = f2;
	while(fptr != NULL)
	{
		if(strcmp(fptr->word, ptr1->word) == 0 && strcmp(fptr->word, ptr2->word) == 0)
		{
			fptr->WFD = (ptr1->WFD + ptr2->WFD)/2;
			fptr = fptr->next;
		}
		else if(strcmp(fptr->word, ptr1->word) == 0|| strcmp(fptr->word, ptr2->word) == 0)
		{
			if(strcmp(fptr->word, ptr1->word) == 0 && strcmp(fptr->word, ptr2->word) > 0)
			{
				ptr2 = ptr2->next;
			}
			else if(strcmp(fptr->word, ptr1->word) == 0 && strcmp(fptr->word, ptr2->word) < 0)
			{
				fptr->WFD = (ptr1->WFD)/2;
				fptr = fptr->next;
			}
			else if(strcmp(fptr->word, ptr1->word) > 0 && strcmp(fptr->word, ptr2->word) == 0)
			{
				ptr1 = ptr1->next;
			}
			else
			{
				fptr->WFD = (ptr2->WFD)/2;
				fptr = fptr->next;
			}
		}
		else
		{
			if(strcmp(fptr->word, ptr1->word) > 0)
				ptr1 = ptr1->next;
			if(strcmp(fptr->word, ptr2->word) > 0)
				ptr2 = ptr2->next;
		}
		//fptr->WFD = (double)fptr->WFD/(double)fhead->totalnodes; // this is not the correct MEAN WFD formula
		//printf("total words is %d\n",fhead->totalnodes);
		//fptr = fptr->next;
	}

	//double result = totalcomputation(f1, f2, fhead);
	//freeNodes(fhead);
	return fhead;
}

int addToArray(int id, comp_result* add, filenode* file1, filenode* file2)
{
	printf("in addto Array method printing before JSD %d\n",file1->head->totalnodes);
	double JSD = createCombined(file1->head, file2->head);
	printf("BEFORE assign total\n");
	unsigned total = file1->totalnodes + file2->totalnodes;
	printf("AFTER assign total\n");
	add[id] = Create(file1->filename, file2->filename, total, JSD);
	printf("THE JSD IS %f, THE FIRST FILENAME IS %s THE SECOND FILENAME IS %s\n",JSD,file1->filename,file2->filename);
	printf("AFTER CREATING COMP RESULT\n");
	return 0;
}

int main(int argc, char **argv)
{
	wordnode* f1 = createNode("hi");
	wordnode* f11 = createNode("there");
	f1->next = f11;
	f1->numoccur = 2;
	f1->totalnodes = 4;
	f1->WFD = 0.5;
	f11->numoccur = 2;
	f11->WFD = 0.5;

	wordnode* f21 = createNode("hi");
	wordnode* f22 = createNode("out");
	wordnode* f23 = createNode("there");
	f21->next = f22;
	f22->next = f23;
	f21->totalnodes = 4;
	f21->numoccur = 2;
	f21->WFD = 0.5;
	f22->numoccur = 1;
	f22->WFD = 0.25;
	f23->numoccur = 1;
	f23->WFD = 0.25;

	wordnode* result = createCombined(f1, f21);
	printLinkedlist(result);
	double JSD = totalcomputation(f1, f21, result);

	freeNodes(f1);
	freeNodes(f21);
	freeNodes(result);
	return EXIT_SUCCESS;
}
