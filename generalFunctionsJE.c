#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "structs.h"
#include "generalFunctions.h"

#define PERMS 0666
#define SERVER_FIFO_NAME "fifo_server_"
#define CLIENT_FIFO_NAME "fifo_client_"


int isNumber(char const *numWorkers){
	int length = strlen(numWorkers);
	for (int i = 0; i < length; i++){
		if ((numWorkers[i] < '0') || (numWorkers[i] > '9')){
			return 0;
		}
	}
	return 1;
}


int digitCounterFunction(int number){
	int count = 0;
	while(number != 0){
		number /= 10;
		count++;
	}
	return count;
}



void checkParameters(int argc, char const **argv){

	//or use getopt()
	if (argc < 5){
		printf("Not enough arguments!\n");
		exit(EXIT_FAILURE);
	}
	else {
		if (strcmp(argv[1], "-d") != 0){
			printf("Wrong parameter for path-file!\n");
			exit(EXIT_FAILURE);
		}
		if (strcmp(argv[3], "-w") != 0){
			printf("Wrong parameter for workers!\n");
			exit(EXIT_FAILURE);
		}
		if (!isNumber(argv[4])){
			printf("Number of workers must be a positive integer\n");
			exit(EXIT_FAILURE);
		}
		
	}
	return ;
}


int countLines(int fileDesc){

	ssize_t nreads;
	char buf[1];
	int lines = 0;

	while ((nreads = read(fileDesc, buf, 1)) > 0){
		if ((buf[0] == '\n')) lines++;
	}
	return lines;
}


char ** getPathsFromFile(int fileDesc, int linesOfFile, int *max){

	ssize_t nread;
	char *line = NULL;
	size_t len = 0;

	//give me a FILE* from fileDescriptor
	FILE *fd = fdopen(fileDesc, "r");
	if (fd == NULL){
		printf("errno = %d\n", errno);
		perror("fdopen");
		exit(EXIT_FAILURE);
	}

	//move pointer at the beggining
	if (fseek(fd, 0, SEEK_SET)){
		printf("errno = %d\n", errno);
		perror("fseek");
		exit(EXIT_FAILURE);
	}

	char **pathArray = (char **) malloc(linesOfFile*sizeof(char *));
	if (pathArray == NULL){
		printf("errno = %d\n", errno);
		perror("malloc");
		exit(EXIT_FAILURE);
	}
	
	int length;
	int i = 0;

	while ((nread = getline(&line, &len, fd)) != -1){
		//no need to check for empty line
		if (line[nread - 1] == '\n'){
			line[nread - 1] = '\0';
		}
		if ((int)(strlen(line)) > *(max)){
			*(max) = (int)(strlen(line));
		}

		length = (int)(strlen(line));

		pathArray[i] = (char *) malloc((length + 1)*sizeof(char));
		if (pathArray[i] == NULL){
			printf("errno = %d\n", errno);
			perror("malloc");
			exit(EXIT_FAILURE);
		}

		strcpy(pathArray[i], line);
		//pathArray[length + 1] = '\0';
		i++;
	}
	
	if (fclose(fd) < 0){
		printf("errno = %d\n", errno);
		perror("fclose at getPathsFromFile");
		exit(EXIT_FAILURE);
	}
	

	if (line != NULL){
		free(line);
		line = NULL;
	}

	return pathArray;
}


char * make_fifo_name(pid_t pid, char *currentName, char *name, int defaultNameSize, int type){

	int j = 0;
	if (type == 0){			//server
		j = snprintf(name, (size_t)(defaultNameSize + digitCounterFunction(pid) + 1), "fifo_server_%ld", (long) pid);
		return name;
	}
	else if (type == 1){	//client
		j = snprintf(name, (size_t)(defaultNameSize + digitCounterFunction(pid) + 1), "fifo_client_%ld", (long) pid);
		return name;
	}
	else if (type == 2){
		name = (char *) malloc(((int)(defaultNameSize + digitCounterFunction(pid) + 2))*sizeof(char));
		if (name == NULL){
			perror("malloc");
			exit(EXIT_FAILURE);
		}
		int i;
		for (i = 0; i < defaultNameSize; i++){
		 	name[i] = currentName[i];
		}
		name[i] = '_';
		i++;
		//printf("--->%s\n", name);
		j = digitCounterFunction(pid);
		for (i = (defaultNameSize + j); i > defaultNameSize; i--){
			int temp = (pid%10);
			name[i] = temp + '0';
			pid /= 10;
		}
		//printf("%s\n", name);
		//name[defaultNameSize + j + 1] = '\0';
		name[defaultNameSize + j + 1] = '\0';
		return name;
	}
	else {
		fprintf(stderr, "Wrong type in make_fifo_name #%d!!\n", type);
		exit(EXIT_FAILURE);
	}
	return NULL;
}


char *toArray(int number){
	int n = digitCounterFunction(number) + 1;
	//int i, temp;
	char *numberArray = (char *)malloc(n*sizeof(char));
	if (numberArray == NULL){
		printf("errno = %d\n", errno);
		perror("malloc");
		exit(EXIT_FAILURE);
	}
	sprintf(numberArray, "%d", number);
	return numberArray;
}


void printCommands(){
	printf("The allowed commands are:\n");
	printf("--> /search [q1] [q2] ... [qN] -d deadline(time)\n");
	printf("--> /maxcount keyword \n");
	printf("--> /mincount keyword\n");
	printf("--> /wc\n");
	printf("--> /exit\n");
	printf("\n");
	return;
}


int countSlashes(char *directory, int maxSize){
	int i;
	//directory[maxSize] = '\0';
	//int length = (int)(strlen(directory)); 
	char *temp;
	temp = (char *)malloc(((int)(strlen(directory)) + 1)*sizeof(char));
	if (temp == NULL){
		printf("errno = %d\n", errno);
		perror("malloc at countSlashes");
		exit(EXIT_FAILURE);
	}
	//temp[0] = '\0';
	strcpy(temp, directory);
	temp[(int)(strlen(temp))] = '\0';
	for (i = (maxSize - 1); i > 0; i--){
		if (temp[i - 1] == '/'){
			continue;
		}
		else{
			if (temp != NULL){
				free(temp);
				temp = NULL;
			}
			return i;
		}
	}
	if (temp != NULL){
		free(temp);
		temp = NULL;
	}
	return 0;
}


int countWords(char *arguments){

	int counter = 0;
	char *token;
	token = strtok(arguments, " \t\n");
	while (token != NULL){
		counter++;
		token = strtok(NULL, " \t\n");
	}
	return counter;
}


int searchArgumentsCheck(char *arguments){

	int wordCounter, length, ret;
	char *token;
	char *temp1 = (char *)malloc((strlen(arguments) + 1)*sizeof(char));
	if (temp1 == NULL){
		printf("errno = %d\n", errno);
		perror("malloc at searchArgumentsCheck (1)");
		exit(EXIT_FAILURE);
	}
	strcpy(temp1, arguments);
	char *temp2 = (char *)malloc((strlen(arguments) + 1)*sizeof(char));
	if (temp2 == NULL){
		printf("errno = %d\n", errno);
		perror("malloc at searchArgumentsCheck (2)");
		exit(EXIT_FAILURE);
	}
	strcpy(temp2, arguments);
	wordCounter = countWords(temp1);
	if (temp1 != NULL){
		free(temp1);
		temp1 = NULL;
	}
	
	//printf("arguments = %s\n", arguments);
	//printf("wordCounter = %d\n", wordCounter);
	if (wordCounter < 2){
		if (temp2 != NULL){
			free(temp2);
			temp2 = NULL;
		}
		return -2;
	}
	return 1;
}


int getDeadline(char *arguments){

	int wordCounter, ret;
	char *token;
	char *temp1 = (char *)malloc((strlen(arguments) + 1)*sizeof(char));
	if (temp1 == NULL){
		printf("errno = %d\n", errno);
		perror("malloc at getDeadline()");
		exit(EXIT_FAILURE);
	}
	strcpy(temp1, arguments);
	char *temp2 = (char *)malloc((strlen(arguments) + 1)*sizeof(char));
	if (temp2 == NULL){
		printf("errno = %d\n", errno);
		perror("malloc at searchArgumentsCheck (2)");
		exit(EXIT_FAILURE);
	}
	strcpy(temp2, arguments);
	wordCounter = countWords(temp1);

	if (temp1 != NULL){
		free(temp1);
		temp1 = NULL;
	}

	token = strtok(temp2, " \t\n");
	for (int i = 0; i < wordCounter; i++){
		if (i == (wordCounter - 1)){
			ret = strtol(token, NULL, 10);

			if (temp2 != NULL){
				free(temp2);
				temp2 = NULL;
			}

			return ret;
		}
		token = strtok(NULL, " \t\n");
	}
	return -1;
}


void deletePostingListWordInfo(PostingListWordInfo *node){
	if (node->next != NULL){
		deletePostingListWordInfo(node->next);
	}
	//if (node->next != NULL){
		//free(node->next);
		//node->next = NULL;
	//}
	//if (node != NULL){
	free(node);
	node = NULL;
	//}
	return;
}


void deletePostingList(ListNode *node){
	if (node->next != NULL){
		deletePostingList(node->next);
	}
	deletePostingListWordInfo(node->wordInfoStruct);
	if (node->nameOfFile != NULL){
		free(node->nameOfFile);
		node->nameOfFile = NULL;
	}
	if (node != NULL){
		free(node);
		node = NULL;
	}
	return;
}


void deleteTrie(Node *currentNode){
	
	if (currentNode->child != NULL){
		deleteTrie(currentNode->child);
	}

	if (currentNode->next != NULL){
		deleteTrie(currentNode->next);
	}
	
	if (currentNode->end != NULL){
		deletePostingList(currentNode->end);
	}

	currentNode->end = NULL;
	currentNode->child = NULL;
	currentNode->next = NULL;
	free(currentNode);
	currentNode = NULL;
	return;
	
}


void perror_exit(char *message){
	perror(message);
	exit(EXIT_FAILURE);
}

/*

int findPosInFifoArray(char *nameOfFifo){

	for (int i = 0; i < globalNumberOfWorkers; i++){
		if ((strcmp(nameOfFifo, fifos[i].clientName) == 0) || (strcmp(nameOfFifo, fifos[i].serverName) == 0)){
			return i;
		}
	}
	return -1;
}


void handler_SIGCHLD(int signo){

	pid_t pid, myPid = getpid(), newPid;
    int status, lengthOfWorkersProgram = 0, lengthOfpid = 0, pos = -1;
    char **args, *workerProgram = "./workerCode";

    lengthOfWorkersProgram = (int)(strlen(workerProgram));
    printf("**********************pid = %d**********************\n", getpid());
    fflush(stdout);

	if ((pid = waitpid(-1, &status, WNOHANG)) != -1){

		char *temp = NULL, *oldClientName;;
		//printf("hiiiiiiii\n");
		args = (char **) malloc(3*sizeof(char *));	//first 2 is for name of program and NULL, 1 is for fathers pid
		if (args == NULL){
			printf("errno = %d\n", errno);
			perror("malloc");
			exit(EXIT_FAILURE);
		}

		//malloc the pointers
		args[0] = (char *) malloc((lengthOfWorkersProgram + 1)*sizeof(char));
		if (args[0] == NULL){
			printf("errno = %d\n", errno);
			perror("malloc");
			exit(EXIT_FAILURE);
		}
		strcpy(args[0], "./workerCode");
		args[0][lengthOfWorkersProgram] = '\0';
		
		lengthOfpid = digitCounterFunction(getpid());
		//printf("%d\n", lengthOfpid);
		args[1] = (char *) malloc((lengthOfpid + 1)*sizeof(char));
		if (args[1] == NULL){
			printf("errno = %d\n", errno);
			perror("malloc");
			exit(EXIT_FAILURE);
		}
		
		char *tempToArray = toArray(myPid);
		strcpy(args[1], tempToArray);
		args[1][lengthOfpid] = '\0';
		//last argument must be NULL
		args[2] = NULL;

		
		for (int i = 0; i < 2; i++){
			printf("-------------------->args[%d] = %s\n", i, args[i]);
		}

		if ((newPid = fork()) == -1){
			printf("errno = %d\n", errno);
			perror("fork call");
			exit(EXIT_FAILURE);
		}
		else{
			execvp(args[0],args);
			printf("errno = %d\n", errno);
			perror("execvp");
			exit(EXIT_FAILURE);
		}



		oldClientName = (char *) malloc(((int)((int)(strlen(CLIENT_FIFO_NAME)) + digitCounterFunction(pid) + 1))*sizeof(char));		//+1 is for '\0'
		if (oldClientName == NULL){
			printf("errno = %d\n", errno);
			perror("malloc");
			exit(EXIT_FAILURE);
		}
		oldClientName = make_fifo_name(pid, " ", oldClientName, (int)(strlen(CLIENT_FIFO_NAME)), 1);
		if (oldClientName == NULL){
			perror("make_fifo_name client");
			exit(EXIT_FAILURE);
		}

		if ((pos = findPosInFifoArray(oldClientName)) == -1){
			printf("I couldn't find match of dead kid's fifo name into fifoArray\n");
			exit(EXIT_FAILURE);
		}
		else{

			//delete previous names
			if (fifos[pos].serverName != NULL){
				free(fifos[pos].serverName);
				fifos[pos].serverName = NULL;
			}
			if (fifos[pos].clientName != NULL){
				free(fifos[pos].clientName);
				fifos[pos].clientName = NULL;
			}

			//making new names
			fifos[pos].serverName = (char *) malloc(((int)((int)strlen(SERVER_FIFO_NAME) + digitCounterFunction(getpid()) + digitCounterFunction(newPid) + 2))*sizeof(char));		//+1 is for '\0'
			if (fifos[pos].serverName == NULL){
				perror("malloc at fifos[i].serverName 1");
				exit(EXIT_FAILURE);
			}
			fifos[pos].serverName = make_fifo_name(getpid(), " ", fifos[pos].serverName, (int)(strlen(SERVER_FIFO_NAME)), 0);
			if (fifos[pos].serverName == NULL){
				perror("make_fifo_name server");
				exit(EXIT_FAILURE);
			}
			temp = (char *) malloc(((int)((int)(strlen(fifos[pos].serverName)) + 1))*sizeof(char));		//+1 is for '\0'
			if (temp == NULL){
				printf("errno = %d\n", errno);
				perror("malloc at temp");
				exit(EXIT_FAILURE);
			}
			strcpy(temp, fifos[pos].serverName);
			fifos[pos].serverName = make_fifo_name(newPid, temp, fifos[pos].serverName, (int)(strlen(fifos[pos].serverName)), 2);
			if (fifos[pos].serverName == NULL){
				perror("make_fifo_name server");
				exit(EXIT_FAILURE);
			}

			fifos[pos].clientName = (char *) malloc(((int)((int)(strlen(CLIENT_FIFO_NAME)) + digitCounterFunction(newPid) + 1))*sizeof(char));		//+1 is for '\0'
			if (fifos[pos].clientName == NULL){
				perror("malloc");
				exit(EXIT_FAILURE);
			}
			fifos[pos].clientName = make_fifo_name(newPid, " ", fifos[pos].clientName, (int)(strlen(CLIENT_FIFO_NAME)), 1);
			if (fifos[pos].clientName == NULL){
				perror("make_fifo_name client");
				exit(EXIT_FAILURE);
			}

			printf("-->'%s'\n", fifos[pos].clientName);
			printf("-->'%s'\n", fifos[pos].serverName);
			
			if (mkfifo(fifos[pos].serverName, PERMS) == -1){
				//printf("---->\n");
				//printf("%s\n", fifos[i].clientName);
				if (errno != EEXIST){
					printf("errno = %d\n", errno);
					perror("receiver: mkfifo (server)");
					printf("Exiting kid %d\n", getpid());
					exit(EXIT_FAILURE);
				}
			}
			//printf("sadasd\n");
			//fflush(stdout);
			//printf("-->'%s'\n", fifos[pos].clientName);
			if (mkfifo(fifos[pos].clientName, PERMS) == -1){
				//printf("'@'%s'@'\n", fifos[i].serverName);
				if (errno != EEXIST){
					printf("errno = %d\n", errno);
					perror("receiver: mkfifo (client)");
					printf("Exiting kid %d\n", getpid());
					exit(EXIT_FAILURE);
				}
			}
			//printf("skataaaaaa\n");
			//fflush(stdout);
			if ((fifos[pos].fd_server_client = open(fifos[pos].serverName, O_WRONLY)) < 0){
				printf("errno = %d\n", errno);
				perror("open fifos[i].serverName");
				exit(EXIT_FAILURE);
			}
			//printf("fifos[%d].serverName = '%s'\n", i, fifos[i].serverName);
			//printf("fifos[%d].clientName = '%s'\n", i, fifos[i].clientName);
			if ((fifos[pos].fd_client_server = open(fifos[pos].clientName, O_RDONLY)) < 0){
				printf("errno = %d\n", errno);
				printf("fifos[%d].clientName = %s\n", pos, fifos[pos].clientName);
				perror("open fifos[i].clientName");
				exit(EXIT_FAILURE);
			}

			if (temp != NULL){
				free(temp);
				temp = NULL;
			}
		}


		if (tempToArray != NULL){
			free(tempToArray);
			tempToArray = NULL;
		}


		for (int k = 0; k < 2; k++){
			if (args[k] != NULL){
				free(args[k]);
				args[k] = NULL;
			}
		}
		if (args != NULL){
			free(args);
			args = NULL;
		}
	}
	

	return;
}
*/