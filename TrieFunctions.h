#ifndef TRIEFUNCTIONS_H
#define TRIEFUNCTIONS_H

#include "structs.h"


Node* createTrie();

int isEmpty(Node *root);

void insert(Node *parentNodePointer, char * word, unsigned int length, int i, char *nameOfFile, int numberOfLine/*, int offsetOfLine*/);

void insertWord(Node *root, char *word, unsigned int length, char *nameOfFile, int numberOfLine/*, int offsetOfLine*/);

void updatePostingList(Node * currentNode, char * nameOfFile, int numberOfLine/*, int offsetOfLine*/);

ListNode * search(Node *root, char *word, unsigned int length);

MaxMinCountStruct *maxFunction(ListNode *list, int maxPathLength);

MaxMinCountStruct *minFunction(ListNode *list, int maxPathLength);

int * pinMapSearch(ListNode *list, Map *map, int sizeOfMap/*, int *exitCounter*/);

void unpinMapSearch(Map *map, int sizeOfMap);



#endif