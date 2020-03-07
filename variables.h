#ifndef VARIABLES_H
#define VARIABLES_H

#include "threadsFunctions.h"
#include "structCrawler.h"
#include "variables.h"

#define POOL_SIZE 50

char symbolicip[50];
char *save_dir;
int serving_threads, num_of_threads, port;

struct hostent *mymachine;


Queue_Head urlQueue;
Exist_Head existHead;
int globalCounter;
int END_ALL;
int downloadedPages;
long int downloadedBytes;

/*
typedef struct {
	int data[POOL_SIZE];
	int start;
	int end;
	int count;
}pool_t;
*/

pthread_mutex_t mtx;
pthread_mutex_t count_mtx;
pthread_cond_t all_serving;
//pool_t  pool;


char *root_dir;
int servedPages;
long int servedBytes;


typedef struct {
	int data[POOL_SIZE];
	int start;
	int end;
	int count;
	//int exit_var;
}pool_t;


pthread_mutex_t mtx;
pthread_mutex_t count_mtx;
pthread_cond_t cond_nonempty;
pthread_cond_t  cond_nonfull;
pool_t  pool;


#endif