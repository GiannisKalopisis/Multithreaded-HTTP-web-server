#ifndef GENERALFUNCTIONSCRAWLER_H
#define GENERALFUNCTIONSCRAWLER_H

#include "structCrawler.h"
#include "generalFunctions.h"


int isNumberCrawler(char const *numWorkers);

void checkParametersCrawler(int argc, char const **argv);

//void perror_exit(char *message);

void initURLQueueHead(Queue_Head *head);

void initExistQueueHead(Exist_Head *head);

int isEmptyExistQueue(Exist_Head *head);

void pushInExistQueue(Exist_Head *head, char *path);

int existInExistQueue(Exist_Head *head, char *path);

int isEmpty(Queue_Head *head);

void push(Queue_Head *head, char *URL, Exist_Head *existHead);

void printAll(Queue_Head *head);

void pop(Queue_Head *head);

char *getFrontURL(Queue_Head *head);

char * makeDir(const char *save_dir);

char * createFilePath(const char *URL, char *saveDir);

char * getURL(char *URL);

char ** getLinksFromFile(char *file);

void printAllQueue(Exist_Head *head);

void deleteExistList(Exist_Head *head);

void getListOfLinks(Exist_Head *new, Exist_Head *head, char *save_dir);

char *makeFileName(Exist_Head *head);


#endif