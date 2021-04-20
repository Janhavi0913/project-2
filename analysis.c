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

/* void freeArray(comp_result* results, int comparisons){
	int k;
	for(k = 0; k < comparisons; k++){
		free(results[k].file1);
		free(results[k].file2);
		free(results[k]);
	}
	free(results);
} */


/* comp_result Create(char* f1, char* f2, unsigned total, double JSD)
{
	struct comp_result* res = (struct comp_result*)malloc(sizeof(struct comp_result));
	res->file1 = f1;
	res->file2 = f2;
	res->totalwords = total;
	res->JSD = JSD;
	return res[0];
} */

double totalcomputation(wordnode* file1, wordnode* file2, wordnode* file)
{
  //printf("In total computation\n");
	double KLD1 = 0, KLD2 = 0, JSD = 0;
	wordnode* ptr = file;
	while(ptr != NULL)
	{
	  //printf("Word in totalfile: %s\n", ptr->word);
		while(strcmp(file1->word, ptr->word) != 0)
			ptr = ptr->next;
		//printf("****************************\n");
		//printf("Total Words in File: %f Word: %s Frequency: %d Mean Word Frequency: %f\n",file1->WFD,file1->word,file1->numoccur,ptr->WFD);
		KLD1 += (file1->WFD*log2((file1->WFD)/(ptr->WFD))); // need to fix how we get mean WFD
		//printf("KLD1 IS %f\n",KLD1);
		//printf("****************************\n");
		ptr = ptr->next;
		if(file1->next != NULL)
		  file1 = file1->next;
		else
		  break;
	}
	wordnode* ptr2 = file;

	while(ptr2 != NULL)
	{			
		while(strcmp(file2->word, ptr2->word) != 0)
		ptr2 = ptr2->next;
		//printf("****************************\n");
		//printf("Total Words in File: %f Word: %s Frequency: %d Mean Word Frequency: %f\n",file2->WFD,file2->word,file2->numoccur,ptr2->WFD);
		KLD2 += (file2->WFD*log2((file2->WFD)/(ptr2->WFD))); // need to fix how we get mean WFD
		//printf("KLD2 IS %f\n",KLD2);
		//	printf("****************************\n");
		ptr2 = ptr2->next;
		if(file2->next != NULL)
		  file2 = file2->next;
		else
		  break;
	}
	JSD = sqrt((0.5*KLD1)+(0.5*KLD2));
	//printf("TOTAL COMPUTATION RESULT IS %f\n",JSD);
	//RETURN JSD WHEN KLD IS FIXED TODO!!!!!
	return JSD;
}

double  createCombined(wordnode* f1, wordnode* f2)
{
  //printf("The word count for file 1 here is %d\n",f1->totalnodes);
	//printf("The word count for file 2 here is %d\n",f2->totalnodes);
	if(f1 == NULL || f2 == NULL)
	{
		if(f1 == NULL && f2 == NULL)
			return 0;
		else{
			return sqrt(0.5);
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
	
	//printf("CREATING NODE WITH HEAD AS %s\n",f1->word);
	fhead = createNode(f1->word);
	//printf("FINISHED CREATING NODE WITH HEAD AS %s\n",fhead->word);
	fhead->numoccur = f1->numoccur;
	//printf("The total words for f1 here is %d\n",f1->totalnodes);
	//printf("The total words for f2 here is %d\n",f2->totalnodes);
	fhead->totalnodes = f1->totalnodes + f2->totalnodes;
	//printf("COMBINED TOTAL OF WORDS IS %d\n", fhead->totalnodes);
	
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
	printf("1File1 is: %s File2 is: %s\n", f1->word, f2->word);
	int x = 0;
	while(fptr != NULL)
	{
	  printf("[%d] FPTR WORD is: %s PTR1 Word is: %s PTR2 Word is: %s*\n",x,fptr->word,ptr1->word,ptr2->word);
	  if(ptr1 == NULL || ptr2 == NULL)
	    {
	      if(ptr1 == NULL && ptr2 == NULL)
		break;
	      else if(ptr1 == NULL)
		{
		  while(ptr2 != NULL && fptr != NULL)
		    {
		      fptr->WFD = (ptr2->WFD)/2;
		      fptr = fptr->next;
		      ptr2 = ptr2->next;
		    }
		  break;
		}
	      else
		{
		  while(ptr1 != NULL && fptr != NULL)
                    {
                      fptr->WFD	= (ptr1->WFD)/2;
                      fptr = fptr->next;
                      ptr1 = ptr1->next;
                    }
		  break;
		}
	    }
	  //printf("Word is: %s\n", fptr->word);
		if(strcmp(fptr->word, ptr1->word) == 0 && strcmp(fptr->word, ptr2->word) == 0)
		{
			fptr->WFD = (ptr1->WFD + ptr2->WFD)/2;
			fptr = fptr->next;
			ptr1 = ptr1->next;
			ptr2 = ptr2->next;
		}
		else if(strcmp(fptr->word, ptr1->word) == 0 || strcmp(fptr->word, ptr2->word) == 0)
		{
			if(strcmp(fptr->word, ptr1->word) == 0 && strcmp(fptr->word, ptr2->word) > 0)
			{
				ptr2 = ptr2->next;
			}
			else if(strcmp(fptr->word, ptr1->word) == 0 && strcmp(fptr->word, ptr2->word) < 0)
			{
				fptr->WFD = (ptr1->WFD)/2;
				fptr = fptr->next;
				ptr1 = ptr1->next;
			}
			else if(strcmp(fptr->word, ptr1->word) > 0 && strcmp(fptr->word, ptr2->word) == 0)
			{
				ptr1 = ptr1->next;
			}
			else
			{
				fptr->WFD = (ptr2->WFD)/2;
				fptr = fptr->next;
				ptr2 = ptr2->next;
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
		x++;
	}
	printf("After while loop and before total computation\n");
	double result = totalcomputation(f1, f2, fhead);
	freeWordNodes(fhead);
	return result;
}

int addToArray(int id, comp_result* add, filenode* file1, filenode* file2)
{
  //printf("this is file1 name in addtoarray %s\n", add[0].file1);
  //printf("this is file2 name in addtoarray %s\n", add[0].file2);
    //printf("in addto Array method printing before JSD %d\n",file1->head->totalnodes);
  printf("File1 = %s File2 = %s\n", file1->filename, file2->filename);
  double JSD = createCombined(file1->head, file2->head);
	//	printf("BEFORE assign total\n");
	unsigned total = file1->totalnodes + file2->totalnodes;
	//printf("AFTER assign total\n");
	//printf("@@this is file1: %s and file2: %s\n", file1->filename, file2->filename);
	//printf("what is id %d\n",id);
	add[id].file1 = file1->filename;
	add[id].file2 = file2->filename;
	add[id].totalwords = total;
	add[id].JSD = JSD;
	//add[id] = Create(file1->filename, file2->filename, total, JSD);
	printf("THE JSD IS %f, THE FIRST FILENAME IS %s THE SECOND FILENAME IS %s\n",JSD,file1->filename,file2->filename);
	printf("AFTER CREATING COMP RESULT\n");
	return 0;
}
