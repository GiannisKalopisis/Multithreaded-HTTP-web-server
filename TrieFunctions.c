#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "structs.h"
#include "generalFunctions.h"
#include "TrieFunctions.h"




Node * createTrie(){
	Node *trieNode = NULL;
	trieNode = (Node *)malloc(sizeof(Node));
	if (trieNode){
		trieNode->letter = ' ';
		trieNode->end = NULL;
		trieNode->child = NULL;
		trieNode->next = NULL;
	}
	else {
		perror("malloc into createTrie()");
		exit(EXIT_FAILURE);
	}
	return trieNode;
}


int isEmpty(Node *root){
	return (root->child == NULL);
}


void insert(Node * parentNodePointer, char * word, unsigned int length, int i, char * nameOfFile, int numberOfLine/*, int offsetOfLine*/){
	while (i < length){
		if (parentNodePointer != NULL){
			parentNodePointer->letter = word[i];
			parentNodePointer->end = NULL;
			parentNodePointer->child = NULL;
			parentNodePointer->next = NULL;
			if (i < (length - 1)){
				parentNodePointer->child = (Node *)malloc(sizeof(Node));
				parentNodePointer = parentNodePointer->child;
			}
		}
		else {
			perror("malloc at insert()");
			exit(EXIT_FAILURE);	
		}
		i++;
	}
	//printf("before updatePostingList and nameOfFile is == '%s'\n", nameOfFile);
	updatePostingList(parentNodePointer, nameOfFile, numberOfLine/*, offsetOfLine*/);
	return;
}


void insertWord(Node *root, char *word, unsigned int length, char *nameOfFile, int numberOfLine/*, int offsetOfLine*/){
	int i = 0;
	Node *currentNode;
	if (!isEmpty(root)){
		currentNode = root->child;
		for (i = 0; i < length ; i++){
			//goes right until the end
			while (currentNode->letter != word[i]){	 
				//move to next neighbor
				if (currentNode->next != NULL) {
					currentNode = currentNode->next;
				}
				else {
					currentNode->next = (Node *)malloc(sizeof(Node));
					if (currentNode->next){
						insert(currentNode->next, word, length, i, nameOfFile, numberOfLine/*, offsetOfLine*/);
					}
					else {
						printf("errno = %d\n", errno);
						perror("Could not allocate memory for new node at insertWord()");
						exit(EXIT_FAILURE);
					}
					return;
				}
			}
			//letter found so go down
			if (i == (length - 1)){	//we are not at the end of the word 
				//update postinList
				updatePostingList(currentNode, nameOfFile, numberOfLine/*, offsetOfLine*/);
			}
			else {
				//if word inserted word stop here
				//insert the rest of the word that we want to insert
				if(currentNode->child != NULL){
					//printf("has child\n");
					currentNode = currentNode->child;
				}
				else {
					currentNode->child = (Node *)malloc(sizeof(Node));
					if (currentNode->child){
						//i + 1 puts the next letter
						//current letter already exists
						insert(currentNode->child, word, length, i + 1, nameOfFile, numberOfLine/*, offsetOfLine*/);
					}
					else {
						printf("errno = %d\n", errno);
						perror("Could not allocate memory for new node at insertWord()");
						exit(EXIT_FAILURE);
					}
					return;
				}
			}	
		}
		
	}
	else {
		//insert the whole word
		root->child = (Node *)malloc(sizeof(Node));
		if (root->child){
			insert(root->child, word, length, i, nameOfFile, numberOfLine/*, offsetOfLine*/);
		}
		else {
			printf("errno = %d\n", errno);
			perror("Could not allocate memory for new node at insertWord()");
			exit(EXIT_FAILURE);
		}
		return;
	}
}


void updatePostingList(Node *currentNode, char *nameOfFile, int numberOfLine/*, int offsetOfLine*/){
	if (currentNode->end != NULL){
		ListNode *list = currentNode->end;
		while (strcmp(list->nameOfFile, nameOfFile) != 0/*list->posNtimes[0] != id*/){
			if (list->next != NULL){
				list = list->next;
			}
			else if (list->next == NULL){
					ListNode *newNode = (ListNode *)malloc(sizeof(ListNode));
					if (newNode != NULL){
						int length = strlen(nameOfFile);
						newNode->nameOfFile = (char *)malloc((length + 1)*sizeof(char));
						if (newNode->nameOfFile == NULL){
							printf("errno = %d\n", errno);
							perror("malloc of newNode->nameOfFile");
							exit(EXIT_FAILURE);
						}
						strcpy(newNode->nameOfFile, nameOfFile);
						//printf("-->newNode->nameOfFile == '%s'\n", nameOfFile);
						//fflush(stdout);
						newNode->wordInfoStruct = (PostingListWordInfo *)malloc(sizeof(PostingListWordInfo));
						if (newNode->wordInfoStruct == NULL){
							printf("errno = %d\n", errno);
							perror("malloc of newNode->wordInfoStruct");
							exit(EXIT_FAILURE);
						}
						newNode->wordInfoStruct->numberOfLine = numberOfLine;
						//newNode->wordInfoStruct->offsetOfLine = offsetOfLine;
						newNode->wordInfoStruct->next = NULL;
						newNode->wordCounterInFile = 1;	
						newNode->next = NULL;
						list->next = newNode;
						//list->wordCounterInFile += 1;
						return;
					}
					else {
						printf("errno = %d\n", errno);
						perror("Could not allocate memory for new ListNode at updatePostingList()");
						exit(EXIT_FAILURE);
					}
			}
			else {
				perror("Last pointer of posting list is not NULL");
				exit(EXIT_FAILURE);
			}
		}
		if (strcmp(list->nameOfFile, nameOfFile) == 0){
			while (list->wordInfoStruct != NULL){
				if (list->wordInfoStruct->numberOfLine == numberOfLine){
					list->wordCounterInFile = list->wordCounterInFile + 1;
					return;
				}
				else {
					list->wordInfoStruct = list->wordInfoStruct->next;
				}
			}
			if (list->wordInfoStruct == NULL){
				list->wordInfoStruct = (PostingListWordInfo *)malloc(sizeof(PostingListWordInfo));
				if (list->wordInfoStruct == NULL){
					printf("errno = %d\n", errno);
					perror("malloc of newWordInfoNode");
					exit(EXIT_FAILURE);
				}
				list->wordInfoStruct->numberOfLine = numberOfLine;
				//list->wordInfoStruct->offsetOfLine = offsetOfLine;
				list->wordInfoStruct->next = NULL;
				list->wordCounterInFile = list->wordCounterInFile + 1;
			}
			return;
		}
		else {
			perror("Something wrong happend with while loop at updatePostingList()");
			exit(EXIT_FAILURE);
		}
	}
	else {
		ListNode *list = (ListNode *)malloc(sizeof(ListNode));
		if (list != NULL){
			int length = strlen(nameOfFile);
			list->nameOfFile = (char *)malloc((length + 1)*sizeof(char));
			if (list->nameOfFile == NULL){
				printf("errno = %d\n", errno);
				perror("malloc of list->nameOfFile");
				exit(EXIT_FAILURE);
			}
			strcpy(list->nameOfFile, nameOfFile);
			//printf("||-->newNode->nameOfFile == '%s'\n", nameOfFile);
			list->wordInfoStruct = (PostingListWordInfo *)malloc(sizeof(PostingListWordInfo));
			if (list->wordInfoStruct == NULL){
				printf("errno = %d\n", errno);
				perror("malloc of newWordInfoNode");
				exit(EXIT_FAILURE);
			}
			list->wordInfoStruct->numberOfLine = numberOfLine;
			//list->wordInfoStruct->offsetOfLine = offsetOfLine;
			list->wordInfoStruct->next = NULL;
			list->wordCounterInFile = 1;
			list->next = NULL;
			currentNode->end = list;
		}
		else {
			printf("errno = %d\n", errno);
			perror("malloc at creating posting list");
			exit(EXIT_FAILURE);
		}	
	}
	return;
}


ListNode * search(Node *root, char *word, unsigned int length){
	int i = 0;
	//int flag;
	//length isn't necessary
	if (isEmpty(root)){
		printf("The Trie is empty. The word: %s doesn't exist into Trie.\n", word);
		return NULL;
	}
	Node *currentNode;
	currentNode = root->child;
	for (i = 0; i<length; i++){
		while (currentNode->letter != word[i]){
			if (currentNode->next != NULL){
				currentNode = currentNode->next;
			}
			else {
				//printf("Word %s doesn't exists at Trie\n", word);
				return NULL;
			}
		}
		if (currentNode->letter == word[i]){
			//printf("------> '%c'\n", word[i]);
			if (i == (length - 1)){
				//word found
				//int flag = 1;
				break;
			}
			if (currentNode->child != NULL){
				currentNode = currentNode->child;
			}
			else {
				return NULL;
			}
		}
		else {
			fprintf(stderr, "ERROR: Something unexpected occured at function insertWord(), at line #%d!!!\n", __LINE__);
			exit(EXIT_FAILURE);
		}
	}
	if (currentNode->end == NULL){
		printf("Word: %s doesn't have posting list!\n", word);
		//exit(EXIT_FAILURE);
		return NULL;
	}
	return currentNode->end;
}



MaxMinCountStruct *maxFunction(ListNode *list, int maxPathLength){

	//printf("maxPathLength is: %d\n", maxPathLength);
	if (list == NULL){
		//printf("CurrentNode cannot be NULL and i can't search for NULL into minFunction()\n");
		return NULL;
	}
	MaxMinCountStruct *maxStruct = (MaxMinCountStruct *)malloc(sizeof(MaxMinCountStruct));
	if (maxStruct == NULL){
		printf("errno = %d\n", errno);
		perror("Couldn't malloc minStruct at minFunction()");
		exit(EXIT_FAILURE);
	}
	maxStruct->fullFilePath = (char *)malloc((maxPathLength + 1)*sizeof(MaxMinCountStruct));
	if (maxStruct->fullFilePath == NULL){
		printf("errno = %d\n", errno);
		perror("Couldn't malloc minStruct->fullFilePath at minFunction()");
		exit(EXIT_FAILURE);
	}
	maxStruct->timesWordExist = 0;
	maxStruct->timesWordExist = list->wordCounterInFile;
	strcpy(maxStruct->fullFilePath, list->nameOfFile);
	list = list->next;
	//printf("into minFunction!!!\n");
	while(list != NULL){
		/*
	 	printf("....list->wordCounterInFile == %d\n", list->wordCounterInFile);
	 	printf("....list->nameOfFile == '%s'\n", list->nameOfFile);
	 	printf("....minStruct->timesWordExist == %d\n", minStruct->timesWordExist);
	 	printf("....minStruct->fullFilePath == %s\n", minStruct->fullFilePath);

		printf("before first if: \n");
		printf("list->wordCounterInFile: %d\n", list->wordCounterInFile);
		printf("minStruct->timesWordExist: %d\n", minStruct->timesWordExist);
		*/
		if ((list->wordCounterInFile == maxStruct->timesWordExist)){	//wordCounterInFile will never be 0 so i dont have problem to strcmp with NULL
			if (list->wordCounterInFile == 0){
				printf("list->wordCounterInFile cannot be 0, error at updatePostingList() detected!\n");
				exit(EXIT_FAILURE);
			}
			else {
				if (strcmp(list->nameOfFile, maxStruct->fullFilePath) < 0){
					//printf("list->nameOfFile is: '%s'\n", list->nameOfFile);
					//printf("list->wordCounterInFile = %d , minStruct->timesWordExist = %d\n", list->wordCounterInFile, minStruct->timesWordExist);
					fflush(stdout);
					strcpy(maxStruct->fullFilePath, list->nameOfFile);
				}
			}
		}
		/*
		printf("before second if: \n");
		printf("list->wordCounterInFile: %d\n", list->wordCounterInFile);
		printf("minStruct->timesWordExist: %d\n", minStruct->timesWordExist);
		*/
		if (list->wordCounterInFile > maxStruct->timesWordExist){
			//printf("///////list->nameOfFile is: '%s'\n", list->nameOfFile);
			fflush(stdout);
			maxStruct->timesWordExist = list->wordCounterInFile;
			strcpy(maxStruct->fullFilePath, list->nameOfFile);
			//maxStruct->fullFilePath[strlen(list->nameOfFile)] = '\0';
		}
		list = list->next;
	}
	//printf("into minFunction and minStruct->fullFilePath is: '%s' and minStruct->timesWordExist is: %d\n", minStruct->fullFilePath, minStruct->timesWordExist);
	return maxStruct;
}


MaxMinCountStruct *minFunction(ListNode *list, int maxPathLength){
	//printf("maxPathLength is: %d\n", maxPathLength);
	if (list == NULL){
		//printf("CurrentNode cannot be NULL and i can't search for NULL into minFunction()\n");
		return NULL;
	}
	MaxMinCountStruct *minStruct = (MaxMinCountStruct *)malloc(sizeof(MaxMinCountStruct));
	if (minStruct == NULL){
		printf("errno = %d\n", errno);
		perror("Couldn't malloc minStruct at minFunction()");
		exit(EXIT_FAILURE);
	}
	minStruct->fullFilePath = (char *)malloc((maxPathLength + 1)*sizeof(MaxMinCountStruct));
	if (minStruct->fullFilePath == NULL){
		printf("errno = %d\n", errno);
		perror("Couldn't malloc minStruct->fullFilePath at minFunction()");
		exit(EXIT_FAILURE);
	}
	minStruct->timesWordExist = 0;
	minStruct->timesWordExist = list->wordCounterInFile;
	strcpy(minStruct->fullFilePath, list->nameOfFile);
	list = list->next;
	//printf("into minFunction!!!\n");
	while(list != NULL){
		/*
	 	printf("....list->wordCounterInFile == %d\n", list->wordCounterInFile);
	 	printf("....list->nameOfFile == '%s'\n", list->nameOfFile);
	 	printf("....minStruct->timesWordExist == %d\n", minStruct->timesWordExist);
	 	printf("....minStruct->fullFilePath == %s\n", minStruct->fullFilePath);

		printf("before first if: \n");
		printf("list->wordCounterInFile: %d\n", list->wordCounterInFile);
		printf("minStruct->timesWordExist: %d\n", minStruct->timesWordExist);
		*/
		if ((list->wordCounterInFile == minStruct->timesWordExist)){	//wordCounterInFile will never be 0 so i dont have problem to strcmp with NULL
			if (list->wordCounterInFile == 0){
				printf("list->wordCounterInFile cannot be 0, error at updatePostingList() detected!\n");
				exit(EXIT_FAILURE);
			}
			else {
				if (strcmp(list->nameOfFile, minStruct->fullFilePath) < 0){
					//printf("list->nameOfFile is: '%s'\n", list->nameOfFile);
					//printf("list->wordCounterInFile = %d , minStruct->timesWordExist = %d\n", list->wordCounterInFile, minStruct->timesWordExist);
					fflush(stdout);
					strcpy(minStruct->fullFilePath, list->nameOfFile);
				}
			}
		}
		/*
		printf("before second if: \n");
		printf("list->wordCounterInFile: %d\n", list->wordCounterInFile);
		printf("minStruct->timesWordExist: %d\n", minStruct->timesWordExist);
		*/
		if (list->wordCounterInFile < minStruct->timesWordExist){
			//printf("///////list->nameOfFile is: '%s'\n", list->nameOfFile);
			fflush(stdout);
			minStruct->timesWordExist = list->wordCounterInFile;
			strcpy(minStruct->fullFilePath, list->nameOfFile);
			//maxStruct->fullFilePath[strlen(list->nameOfFile)] = '\0';
		}
		list = list->next;
	}
	//printf("into minFunction and minStruct->fullFilePath is: '%s' and minStruct->timesWordExist is: %d\n", minStruct->fullFilePath, minStruct->timesWordExist);
	return minStruct;
}


int * pinMapSearch(ListNode *list1, Map *map, int sizeOfMap/*, int *exitCounter*/){
	ListNode *list = list1;
	PostingListWordInfo *posting;
	int *printToFile = (int *)malloc(sizeOfMap*sizeof(int));
	if (printToFile == NULL){
		printf("errno = %d\n", errno);
		perror("malloc printToFile");
		exit(EXIT_FAILURE);
	}
	for (int i = 0; i < sizeOfMap; i++){
		printToFile[i] = 0;
	}
	/*
	if (list != NULL){
		(*exitCounter)++;
	}*/
	while (list != NULL){
		posting = list->wordInfoStruct;
		for (int i = 0; i < sizeOfMap; i++){
			if (strcmp(list->nameOfFile, map[i].filePath) == 0){
				///
				///
				///
				while (/*list->wordInfoStruct*/posting != NULL){
					//printf("___map[i].searchQuery[list->wordInfoStruct->numberOfLine] = %d, '%s'\n", map[i].searchQuery[list->wordInfoStruct->numberOfLine], list->nameOfFile);
					map[i].searchQuery[list->wordInfoStruct->numberOfLine] = 1;
					printToFile[i] = 1;
					//list->wordInfoStruct = list->wordInfoStruct->next;
					posting = posting->next;
				}
				break;
			}
		}
		list = list->next;
	}
	return printToFile;
}


void unpinMapSearch(Map *map, int sizeOfMap){
	for (int i = 0; i < sizeOfMap; i++){
		for (int j = 0; j < map[i].numLines; j++){
			map[i].searchQuery[j] = 0;
		}
	}
	return;
}


