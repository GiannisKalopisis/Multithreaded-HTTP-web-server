#ifndef GENERALFUNCTIONS_H
#define GENERALFUNCTIONS_H


#include "structs.h"
/*
extern char **pathArray;
extern FifoNames *fifos;
extern int globalNumberOfWorkers;
*/

int isNumber(char const *numWorkers);

int digitCounterFunction(int number);

void checkParameters(int argc, char const **argv);

int countLines(int fileDesc);

char ** getPathsFromFile(int fileDesc, int linesOfFile, int *max);

char * make_fifo_name(pid_t pid, char *name2, char *name, int defaultNameSize, int type);

char *toArray(int number);

void printCommands();

int countSlashes(char *directory, int maxSize);

int searchArgumentsCheck(char *arguments);

int countWords(char *arguments);

int getDeadline(char *arguments);

void deletePostingListWordInfo(PostingListWordInfo *node);

void deletePostingList(ListNode *node);

void deleteTrie(Node *currentNode);

void perror_exit(char *message);

//int findPosInFifoArray(char *nameOfFifo);

//void handler_SIGCHLD(int signo);

#endif