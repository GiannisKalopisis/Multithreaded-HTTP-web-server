#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libgen.h>
#include "structCrawler.h"
#include "generalFunctionsCrawler.h"
#include "generalFunctions.h"


#define DIR1 "./%s"
#define DIR2 ".%s"



int isNumberCrawler(char const *numWorkers){
	int length = strlen(numWorkers);
	for (int i = 0; i < length; i++){
		if ((numWorkers[i] < '0') || (numWorkers[i] > '9')){
			return 0;
		}
	}
	return 1;
}



void checkParametersCrawler(int argc, char const **argv){

	if (argc < 12){
		fprintf(stderr, "Not enough arguments!\n");
		exit(EXIT_FAILURE);
	}
	else{

		//checking for -h, -p, -c, -t, -d parameters
		if ((strcmp(argv[1], "-h") != 0)){
			fprintf(stderr, "Wrong parameter for host_or_IP!\n");
			exit(EXIT_FAILURE);
		}
		if ((strcmp(argv[3], "-p") != 0)){
			fprintf(stderr, "Wrong parameter for port!\n");
			exit(EXIT_FAILURE);
		}
		if ((strcmp(argv[5], "-c") != 0)){
			fprintf(stderr, "Wrong parameter for command_port!\n");
			exit(EXIT_FAILURE);
		}
		if ((strcmp(argv[7], "-t") != 0)){
			fprintf(stderr, "Wrong parameter for num_of_threads!\n");
			exit(EXIT_FAILURE);
		}
		if ((strcmp(argv[9], "-d") != 0)){
			fprintf(stderr, "Wrong parameter for save_dir!\n");
			exit(EXIT_FAILURE);
		}

		//checking if parameters are numbers
		if (!isNumberCrawler(argv[4])){
			fprintf(stderr, "Port must be a positive integer!\n");
			exit(EXIT_FAILURE);
		}

		if (!isNumberCrawler(argv[6])){
			fprintf(stderr, "Command_port must be a positive integer!\n");
			exit(EXIT_FAILURE);
		}

		if (!isNumberCrawler(argv[8])){
			fprintf(stderr, "Num_of_threads must be a positive integer!\n");
			exit(EXIT_FAILURE);
		}

		//checking if service and command ports are the same
		if (strcmp(argv[4], argv[6]) == 0){
			fprintf(stderr, "Port and Command_port must be different positive integers\n");
			exit(EXIT_FAILURE);
		}

	}

	return;
}

/*
void perror_exit(char *message){
	perror(message);
	exit(EXIT_FAILURE);
}
*/


void initURLQueueHead(Queue_Head *head){

	head->queue_size = 0;
	head->Front = NULL;
	head->Last = NULL;
	return;
}


int isEmpty(Queue_Head *head){
	return head->queue_size == 0;
}


void push(Queue_Head *head, char *URL, Exist_Head *existHead){

	int lengthOfURL = 0;
	lengthOfURL = strlen(URL);
	
	if (head->Front == NULL){
		if (!existInExistQueue(existHead, URL)){
			head->queue_size++;
			//printf("%s\n", URL);
			pushInExistQueue(existHead, URL);

			head->Front = (URL_Queue *)malloc(sizeof(URL_Queue));
			if (head->Front == NULL){
				perror_exit("malloc");
			}
			head->Last = head->Front;
			head->Front->url = (char *)malloc((lengthOfURL + 1)*sizeof(char));
			if (head->Front->url == NULL){
				perror_exit("malloc");
			}
			strcpy(head->Front->url, URL);
			//printf("head->Front->url == '%s'\n", head->Front->url);
			//head->Front->serving = 0;
			head->Front->next = NULL;
		}
	}
	else {
		if (!existInExistQueue(existHead, (char *)URL)){

			head->queue_size++;
			pushInExistQueue(existHead, URL);

			head->Last->next = (URL_Queue *)malloc(sizeof(URL_Queue));
			if (head->Last->next == NULL){
				perror_exit("malloc");
			}
			head->Last->next->url = (char *)malloc((lengthOfURL + 1)*sizeof(char));
			if (head->Last->next->url == NULL){
				perror_exit("malloc");
			}
			strcpy(head->Last->next->url, URL);
			//printf("head->Last->next->url == '%s'\n", head->Last->next->url);
			head->Last->next->next = NULL;
			head->Last = head->Last->next;
			//head->Last->serving = 0;
		}
	}
	return;
}


void printAll(Queue_Head *head){

	URL_Queue *newnext = head->Front;
	while (newnext != NULL){
		printf("printAll.... '%s'\n", newnext->url);
		newnext = newnext->next;
	}
	return;
}


void pop(Queue_Head *head){

	
	//printf("insert pop\n");
	head->queue_size = head->queue_size - 1;

	//printf("head->Front->url == '%s'\n", head->Front->url);
	free(head->Front->url);
	head->Front->url = NULL;
	//printf("just free head->Front->url\n");

	URL_Queue *newFront = head->Front;

	head->Front = head->Front->next;
	free(newFront);
	newFront = NULL;
	//printf("just free head->Front->next\n");

	//printf("new head->Front->url == '%s'\n", head->Front->url);
	//printf("ok\n");
	return;
}


char *getFrontURL(Queue_Head *head){
	//printf("-->%s\n", head->Front->url);
	//printf("ok\n");
	if (head->queue_size >0){
		//printf("head->queue_size = %d\n", head->queue_size);
		return head->Front->url;
	}
	else{
		//printf("ok2\n");
		return NULL;
	}
}


char * makeDir(const char *save_dir){

	char *buf = NULL;
	int lengthOfDir = 0;
	lengthOfDir = strlen(save_dir);
	if ((save_dir[0] == '.') && (save_dir[1] == '/')){
		// ./mpla
		buf = (char *)malloc((lengthOfDir + 1)*sizeof(char));
		if (buf == NULL){
			perror_exit("malloc");
		}
		strcpy(buf, save_dir);
	}
	else if (save_dir[0] == '/'){
		// /mpla
		buf = (char *)malloc((lengthOfDir + 1 + 1)*sizeof(char));
		if (buf == NULL){
			perror_exit("malloc");
		}
		snprintf(buf, (lengthOfDir + 1 + 1), DIR2, save_dir);
	}
	else {
		// .mpla or mpla 
		buf = (char *)malloc((lengthOfDir + 1 + 2)*sizeof(char));
		if (buf == NULL){
			perror_exit("malloc");
		}
		snprintf(buf, (lengthOfDir + 1 + 2), DIR1, save_dir);
		//printf("buf == %s\n", buf);
	}

	struct stat st = {0};
	if (stat(buf, &st) < 0){
		if (mkdir(buf, 0777) < 0){
			if (errno == EEXIST){
				//printf("I have to make a new directory!\n");
			}
			perror_exit("mkdir");
		}
	}

	//printf("''%s''\n", buf);

	return buf;
}


char * createFilePath(const char *URL, char *saveDir){

	char *dname, *bname, *bnameOfDir;
	char *path, *dirPath;

	bname = basename((char *)URL);
	//printf("basename == '%s'\n", bname);
	dname = dirname((char *)URL);
	//printf("dirname == '%s'\n", dname);
	bnameOfDir = basename(dname);
	//printf("basenameOfDir == '%s'\n", bnameOfDir);

	dirPath = (char *)malloc((strlen(saveDir) + 1 + strlen(bnameOfDir) + 1)*sizeof(char));
	if (dirPath == NULL){
		perror_exit("malloc");
	}

	dirPath[0] = '\0';

	strcat(dirPath, saveDir);
	strcat(dirPath, "/");
	strcat(dirPath, bnameOfDir);

	struct stat st = {0};
	if (stat(dirPath, &st) == -1) {
		if (mkdir(dirPath, 0777) == -1){
			if (errno != EEXIST){
				perror_exit("mkdir");
			}
		}
	}

	path = (char *)malloc((strlen(saveDir) + 1 + strlen(bnameOfDir) + 1 + strlen(bname) + 1)*sizeof(char));
	if (path == NULL){
		perror_exit("malloc");
	}

	path[0] = '\0';

	strcat(path, saveDir);
	strcat(path, "/");
	strcat(path, bnameOfDir);
	strcat(path, "/");
	strcat(path, bname);

	//printf("final path == '%s'\n", path);

	if (dirPath != NULL){
		free(dirPath);
		dirPath = NULL;
	}

	return path;
}


char * getURL(char *URL){

	char *finalURL;
	char *dname, *bname, *bnameOfDir;

	//printf("into getURL == '%s'\n", URL);

	bname = basename((char *)URL);
	//printf("basename == '%s'\n", bname);
	dname = dirname((char *)URL);
	//printf("dirname == '%s'\n", dname);
	bnameOfDir = basename(dname);
	//printf("basenameOfDir == '%s'\n", bnameOfDir);
						   /* "./" */
	finalURL = (char *)malloc((2 + strlen(bnameOfDir) + 1 + strlen(bname) + 1)*sizeof(char));
	if (finalURL == NULL){
		perror_exit("malloc");
	}

	finalURL[0] = '\0';

	//strcat(finalURL, ".");
	strcat(finalURL, "/");
	strcat(finalURL, bnameOfDir);
	strcat(finalURL, "/");
	strcat(finalURL, bname);

	//printf("final url to put in queue is: '%s'\n", finalURL);

	return finalURL;
}



char ** getLinksFromFile(char *file){

	char **paths = NULL, *ret, *file1 = file, *token, *token2, path[50];
	int counter = 0;

	memset(path, '\0', sizeof(path));

	while (1){
		ret = strstr(file1, "<a href=\"../site");
		if (ret != NULL){
			//file2 = ret;
			//token = strtok(file2, ">");
			//printf("%s\n", token);
			for (int i = 0; i < 49; ++i)
			{
				path[i] = *(ret + i);
			}
			file1 = ret + 10;
			//printf("%d\n", (int)(file1));
			token = strtok(path, "\"");
			token = strtok(NULL, "\"");
			//printf("ret == %s\n", ret);
			//printf("token == '%s'\n", token);

			paths = (char **)realloc(paths, (counter + 1)*sizeof(*paths));
			if (paths == NULL){
				perror_exit("realloc");
			}

			paths[counter] = (char *)malloc((strlen(token))*sizeof(char));
			if (paths[counter] == NULL){
				perror_exit("malloc");	
			}

			memset(paths[counter], '\0', sizeof(paths[counter]));			
			for (int i = 0; i < (strlen(token) - 1); i++){
				paths[counter][i] = token[i+1];
				paths[counter][i+1] = '\0';
			}
			//printf("paths[%d] == '%s'\n", counter, paths[counter]);
			counter++;
		}
		else{
			break;
		}
		//ret = ret + file1;
	}
	//printf("----->>>got links\n");
	

	return paths;
}


void initExistQueueHead(Exist_Head *head){

	head->queue_size = 0;
	head->First = NULL;
	return;
}


int isEmptyExistQueue(Exist_Head *head){
	return head->queue_size == 0;
}


int existInExistQueue(Exist_Head *head, char *path){

	exist_Queue *current = head->First;

	while (current != NULL){
		if (strcmp(current->myPath, path) == 0){
			//printf("---------------------current->myPath == '%s' and  path == '%s'---------------------\n", current->myPath, path);
			return 1;
		}
		else if (current->myPath[0] == path[1]){
			int i = 1;
			int length;
			if (strlen(current->myPath) > strlen(path)){
				length = strlen(path);
			}
			else {
				length = strlen(current->myPath);
			}
			while (i < length){
				if (current->myPath[i-1] == path[i]){
					i++;
				}
				else break;
			}
			if (i == length) return 1;
		}
		current = current->next;
	}
	return 0;
}


void pushInExistQueue(Exist_Head *head, char *path){

	head->queue_size++;

	exist_Queue *current = head->First;
	int length = strlen(path);

	if (current != NULL){		
		while (current->next != NULL){
			current = current->next;
		}

		current->next = (exist_Queue *)malloc(sizeof(exist_Queue));
		if (current->next == NULL){
			perror_exit("malloc");
		}

		current->next->myPath = (char *)malloc((length + 1)*sizeof(char));
		if (current->next->myPath == NULL){
			perror_exit("malloc");
		}

		strcpy(current->next->myPath, path);
		current->next->next = NULL;
		//printf("just put %s in queue\n", current->next->myPath);
	}
	else{

		current = (exist_Queue *)malloc(sizeof(exist_Queue));
		if (current == NULL){
			perror_exit("malloc");
		}

		current->myPath = (char *)malloc((length + 1)*sizeof(char));
		if (current->myPath == NULL){
			perror_exit("malloc");
		}
		strcpy(current->myPath, path);
		current->next = NULL;
		//printf("just put %s in queue\n", current->myPath);
		head->First = current;

	}
	return;
}



void printAllQueue(Exist_Head *head){

	exist_Queue *newnext = head->First;
	while (newnext != NULL){
		printf("printAll.... '%s'\n", newnext->myPath);
		newnext = newnext->next;
	}
	return;
}


void getListOfLinks(Exist_Head *new, Exist_Head *head, char *save_dir){

	exist_Queue *current = head->First;
	char *path, *dname, *bname;

	while (current != NULL){
		dname = dirname(current->myPath);
		bname = basename(dname);
		path = (char *)malloc((strlen(save_dir) + strlen(dname) + 1)*sizeof(char));
		if (path == NULL){
			perror_exit("malloc");
		}
		path[0] = '\0';
		strcat(path, save_dir);
		strcat(path, "/");
		strcat(path, bname);
		//printf("path == '%s'\n", path);
		if (!existInExistQueue(new, path)){
			pushInExistQueue(new, path);
		}
		current = current->next;
	}
	return;
}


void deleteExistList(Exist_Head *head){

	exist_Queue *current = head->First;
	exist_Queue *next;

	while (current != NULL){
		next = current->next;
		free(current->myPath);
		current->myPath = NULL;
		free(current);
		current = NULL;
		current = next;
	}

	return;
}


char *makeFileName(Exist_Head *head){

	exist_Queue *current = head->First;
	char *fileName = (char *)malloc((strlen("directoryfile") + 1)*sizeof(char));
	fileName[0] = '\0';
	strcat(fileName, "directoryfile");
	FILE *fp;
	fp = fopen(fileName, "w+");
	if (fp == NULL){
		perror_exit("fopen");
	}

	while (current != NULL){
		fprintf(fp, "%s\n", current->myPath);
		current = current->next;
	}
	fclose(fp);


	return fileName;
}