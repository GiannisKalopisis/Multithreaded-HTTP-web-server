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
#include <sys/timeb.h>
#include <dirent.h>
#include <time.h>
#include "generalFunctionsJE.h"
#include "structs.h"
#include "TrieFunctions.h"



#define PERMS 0666
#define SERVER_FIFO_NAME "fifo_server_"
#define CLIENT_FIFO_NAME "fifo_client_"
#define LOG_FILE_NAME "./log/WORKER_"
#define READ 0
#define WRITE 1


int main(int argc, char const *argv[]){

	int fileDesc/*, lineOffset = 0*/, i, lineCounterForDocuments = 0, tempLineCounter, readArgs = 0, j, numberOfWords = 0, exitCounter = 0;
	unsigned int totalCharsOfFiles = 0, totalWordsOfFiles = 0, totalLinesOfFile = 0, length;
	FifoNames fifos;
	DIR *dirPtr;
	struct dirent *direntP;
	char *token, *line = NULL, *tempPath, *tempPathOfFile, *templine;
	size_t len = 0;
	ssize_t nread;
	int count = 0, readQueryLength = 0, fd_server_open, readQuery = 0, nwrite = 0, fd_client_open = 0;//, tempCount2 = 0;
	int countLinesToSendForSearch = 0, logFileDescriptor;
	char *logFilePath = NULL, *tempMin = NULL, *tempLinePrint;
	long long int timestamp_msec_start, timestamp_msec_end, timestamp_msec_final;
	struct timeb timer_msec;

	//passingArgumentsStruct passingArguments;
	sendStruct argStruct;
	passingQueryLength QueryLength;
	wc_Struct wcStruct;
	MaxMinCountStruct *MaxMinQueryStruct;
	MaxMinSendStruct maxMinSendStruct;
	startingSearchStruct startingSearch;
	searchInfoStruct searchInfo;
	
	
	printf("Into WORKER #%d\n", getpid());
	
	
	
	struct stat st = {0};
	if (stat("./log", &st) == -1) {
		if (mkdir("./log", 0777) == -1){
			if (errno != EEXIST){
				printf("errno = %d\n", errno);
				perror("Creating dir");
				exit(EXIT_FAILURE);
			}
		}
	}

	logFilePath = (char *)malloc(((int)((int)(strlen(LOG_FILE_NAME)) + digitCounterFunction(getpid()) + strlen(".txt") + 1))*sizeof(char));
	if (logFilePath == NULL){
		printf("errno = %d\n", errno);
		printf("WORKER #%d\n", getpid());
		perror("malloc at logFilePath");
		exit(EXIT_FAILURE);
	}
	logFilePath[0] = '\0';
	strcat(logFilePath, LOG_FILE_NAME);
	char *tempToArray = toArray(getpid());
	strcat(logFilePath, tempToArray);
	strcat(logFilePath, ".txt");
	//logFilePath[strlen(LOG_FILE_NAME) + digitCounterFunction(getpid()) + 4 + 1] = '\0';
	//mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH ;
	//printf("logFilePath = '%s'\n", logFilePath);
	
	if ((logFileDescriptor = open ( logFilePath , O_WRONLY | O_CREAT | O_TRUNC , 0666 )) == -1){
		printf("errno = %d\n", errno);
		printf("WORKER #%d and logfilePath = %s\n", getpid(), logFilePath);
		perror("Creating logFile");
		exit(EXIT_FAILURE);
	}
	
	FILE *fd_log_file = fdopen(logFileDescriptor, "w");
	if (fd_log_file == NULL){
		printf("errno = %d\n", errno);
		perror("fdopen");
		exit(EXIT_FAILURE);
	}


	if (tempToArray != NULL){
		free(tempToArray);
		tempToArray = NULL;
	}
	//printf("hi i am child with process id: %d and parent id: %d\n", getpid(), getppid());
	//fflush(stdout);

	//fatherPID = (int) strtol(argv[argc-2], NULL, 10);
	/*
	printf("strlen(SERVER_FIFO_NAME) = %ld\n", strlen(SERVER_FIFO_NAME));
	printf("digitCounterFunction(atoi(argv[argc-1])) = %d\n", digitCounterFunction(atoi(argv[argc-1])));
	printf("digitCounterFunction(getpid()) = %d, %d\n", digitCounterFunction(getpid()), getpid());
	printf("+2\n");
	*/
	fifos.serverName = (char *) malloc(((int) strlen(SERVER_FIFO_NAME) + digitCounterFunction(atoi(argv[argc-1])) + digitCounterFunction(getpid()) + 2)*sizeof(char));		//+1 is for '\0'
	if (fifos.serverName == NULL){
		printf("errno = %d\n", errno);
		perror("malloc at fifos.serverName");
		exit(EXIT_FAILURE);
	}
	fifos.serverName = make_fifo_name(atoi(argv[argc-1]), fifos.serverName, fifos.serverName, (int)(strlen(SERVER_FIFO_NAME)), 0);
	if (fifos.serverName == NULL){
		perror("make_fifo_name server");
		exit(EXIT_FAILURE);
	}
	fifos.serverName = make_fifo_name(getpid(), fifos.serverName, fifos.serverName, (int)(strlen(fifos.serverName)), 2);
	if (fifos.serverName == NULL){
		perror("make_fifo_name server");
		exit(EXIT_FAILURE);
	}

	fifos.clientName = (char *) malloc(((int)(strlen(CLIENT_FIFO_NAME) + digitCounterFunction(getpid()) + 1))*sizeof(char));		//+1 is for '\0'
	if (fifos.clientName == NULL){
		printf("errno = %d\n", errno);
		perror("malloc at fifos.clientName");
		exit(EXIT_FAILURE);
	}
	fifos.clientName = make_fifo_name(getpid(), fifos.serverName, fifos.clientName, (int)(strlen(CLIENT_FIFO_NAME)), 1);
	if (fifos.clientName == NULL){
		perror("make_fifo_name client");
		exit(EXIT_FAILURE);
	}

	//printf("'%s'\n", fifos.serverName);
	if (mkfifo(fifos.serverName, 0644) == -1){
		//printf("'#'%s'#'\n", fifos.clientName);
		if (errno != EEXIST){
			printf("errno = %d\n", errno);
			perror("receiver: mkfifo (server)");
			printf("Exiting kid %d\n", getpid());
			exit(EXIT_FAILURE);
		}
	}	
	if (mkfifo(fifos.clientName, 0666) == -1){
		//printf("....>\n");
		//printf("%s\n", fifos.serverName);
		if (errno != EEXIST){
			printf("errno = %d\n", errno);
			perror("receiver: mkfifo (client)");
			printf("Exiting kid %d\n", getpid());
			exit(EXIT_FAILURE);
		}
	}

	//printf("fifos.clientName = %s\n", fifos.clientName);
	//printf("fifos.serverName = %s\n", fifos.serverName);
	//printf("fifos[%d].serverName = '''%s'''\n", getpid(), fifos.serverName);
	//printf("fifos[%d].clientName = '''%s'''\n", getpid(), fifos.clientName);

	if ((fd_server_open = open(fifos.serverName, O_RDONLY)) < 0){
		printf("errno = %d\n", errno);
		printf("fifos.serverName = %s\n", fifos.serverName);
		perror("open fifos.serverName");
		exit(EXIT_FAILURE);
	}
	if ((fd_client_open = open(fifos.clientName, O_WRONLY)) < 0){
		printf("errno = %d\n", errno);
		printf("fifos.clientName = %s\n", fifos.clientName);
		perror("open fifos.clientName");
		exit(EXIT_FAILURE);
	}

	if ((readArgs = read(fd_server_open, &argStruct, sizeof(argStruct))) < 0){
		printf("errno = %d\n", errno);
		perror("read argStruct");
		exit(EXIT_FAILURE);
	}
	//printf("made fifos into worker\n");
	fflush(stdout);


	//printf("send arguments: __%d__\n", argStruct.numOfArguments);
	/*char Arg[argStruct.maxDirSize];
	for (int i = 0; i < argStruct.maxDirSize; ++i){
		Arg[i] = '/';
	}*/
	//printf("i made the fucking fifos into worker #%d\n", getpid());
	char **Directories = (char **)malloc(argStruct.numOfArguments*sizeof(char *));
	if (Directories == NULL){
		printf("errno = %d\n", errno);
		perror("malloc at Directories");
		exit(EXIT_FAILURE);
	}
	for (int i = 0; i < argStruct.numOfArguments; i++)
	{
		Directories[i] = (char *)malloc((argStruct.maxDirSize)*sizeof(char));
		if (Directories[i] == NULL){
			printf("errno = %d\n", errno);
			printf("i = %d\n", i);
			perror("malloc at Directories[i]");
			exit(EXIT_FAILURE);
		}
		//printf("(%s)\n", argv[i]);
		if ((readArgs = read(fd_server_open, Directories[i], argStruct.maxDirSize)) < 0){
			printf("errno = %d\n", errno);
			printf("i : %d\n", i);
			perror("read Arg");
			exit(EXIT_FAILURE);
		}
		Directories[i][countSlashes(Directories[i], argStruct.maxDirSize)] = '\0';
		//printf("Directories == '%s'\n", Directories[i]);
		//printf("Directories == '%s'\n", Directories[i]);
		
		if ((dirPtr = opendir(Directories[i])) == NULL){
			printf("errno = %d\n", errno);
			printf("i = %d\n", i);
			perror("Cannot open Directories[i]");
			exit(EXIT_FAILURE);
		}
		while ((direntP = readdir(dirPtr)) != NULL){
			if ((strcmp(direntP->d_name, ".") == 0) || (strcmp(direntP->d_name, "..") == 0)){
				continue;
			}
			count++;
			//printf("count is: %d\n", count);
		}
		if (closedir(dirPtr) == -1){
			printf("errno = %d\n", errno);
			printf("i = %d\n", i);
			perror("Cannot close Directories[i]");
			exit(EXIT_FAILURE);
		}
		//printf("AFTER: Directories[%d] == '%s'\n", i, Directories[i]);
	}


	//printf("total files are: %d\n", count);
	//printf("count before malloc of map is: %d\n", count);
	Map *map = (Map *)malloc(count*sizeof(Map));
	if (map == NULL){
		printf("errno = %d\n", errno);
		perror("malloc at map");
		exit(EXIT_FAILURE);
	}
	int tempCount2 = count;
	//int linesArray[argc-2];

	



	//printf("2) into WORKER #%d\n", getpid());
	//printf("fifos.serverName %d '@'%s'@'\n", getpid(), fifos.serverName);
	//printf("fifos.clientName %d '@'%s'@'\n", getpid(), fifos.clientName);

	Node *root = createTrie();
	//printf("after createTrie\n");
	//fflush(stdout);

	//int catalogCounter = 0;
	int docCounterInCatalog = 0;

	//printf("count = %d\n", count);
	count = 0;

	int maxPathLength = 0;
	//printf("before for gamwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww\n");

	for (i = 0; i < argStruct.numOfArguments; i++){
		//printf("into for and i is: %d\n", i);
		//fflush(stdout);
		if ((dirPtr = opendir(Directories[i])) == NULL){
			printf("errno = %d\n", errno);
			printf("i = %d\n", i);
			perror("Cannot open Directories[i]");
			exit(EXIT_FAILURE);
		}
		tempPath = (char *)malloc(((int)(strlen(Directories[i]) + 1))*sizeof(char));
		if (tempPath == NULL){
			printf("errno = %d\n", errno);
			printf("i = %d\n", i);
			perror("malloc at tempPath");
			exit(EXIT_FAILURE);
		}
		//printf("into for mpeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee\n");

		errno = 0;
		strcpy(tempPath, Directories[i]);
		while ((direntP = readdir(dirPtr)) != NULL){
			//printf("into big whileeeeeeeeeeeee\n");
			//printf("------------------------------------------------\n");
			//printf ( "WORKER (%d): inode %d of the entry %s \n" , getpid(),(int) direntP->d_ino , direntP->d_name );
			//printf("into strcmp and '%s'\n", direntP->d_name);
			if ((strcmp(direntP->d_name, ".") == 0) || (strcmp(direntP->d_name, "..") == 0)){
				//printf("into strcmp and '%s'\n", direntP->d_name);
				continue;
			}
			tempPathOfFile = (char *)malloc(((int)(strlen(tempPath) + strlen(direntP->d_name) + 2))*sizeof(char));	//2 is for / and '\0'
			if (tempPathOfFile == NULL){
				printf("errno = %d\n", errno);
				perror("malloc at tempPathOfFile");
				exit(EXIT_FAILURE);
			}
			strcpy(tempPathOfFile, tempPath);
			strcat(tempPathOfFile, "/");
			strcat(tempPathOfFile, direntP->d_name);
			//printf("tempPathOfFile = %s\n", tempPathOfFile);

			if ((fileDesc = open(tempPathOfFile, O_RDONLY, PERMS)) == -1){
				printf("errno = %d\n", errno);
				printf("Current directory is: %s\n", direntP->d_name);
				perror("Opening file of directory");
				exit(EXIT_FAILURE);
			}

			//create map of documents
			tempLineCounter = countLines(fileDesc);
			
			map[count].filePath = (char *)malloc(((int)(strlen(tempPathOfFile) + 1))*sizeof(char));
			if (map[count].filePath == NULL){
				printf("errno = %d\n", errno);
				printf("Current path is: %s\n", tempPathOfFile);
				perror("malloc at map[count].filePath");
				exit(EXIT_FAILURE);
			}

			strcpy(map[count].filePath, tempPathOfFile);
			map[count].numLines = tempLineCounter;
			map[count].text = (char **)malloc(tempLineCounter*sizeof(char *));
			if (map[count].text == NULL){
				printf("errno = %d\n", errno);
				printf("Current path is: %s\n", tempPathOfFile);
				perror("malloc at map[count].text");
				exit(EXIT_FAILURE);	
			}

			map[count].searchQuery = (int *)malloc(tempLineCounter*sizeof(int));
			if(map[count].searchQuery == NULL){
				printf("errno = %d\n", errno);
				printf("Current path is: %s\n", tempPathOfFile);
				perror("malloc of searchQuery");
				exit(EXIT_FAILURE);
			}
			for (j = 0; j < tempLineCounter; j++){
				map[count].searchQuery[j] = 0;
			}


			if (((int)strlen(tempPathOfFile)) > maxPathLength){
				maxPathLength = (int)(strlen(tempPathOfFile));
			}

			//move pointer at the beggining
			if (lseek(fileDesc, 0, SEEK_SET)){
				printf("errno = %d\n", errno);
				perror("lseek");
				exit(EXIT_FAILURE);
			}


			FILE *fd = fdopen(fileDesc, "r");
			if (fd == NULL){
				printf("errno = %d\n", errno);
				perror("fdopen");
				exit(EXIT_FAILURE);
			}
			
			//getline takes '\n' also 
			while ((nread = getline(&line, &len, fd)) != -1){
				//printf("line of file '%s' is: %s", tempPathOfFile, line);
				if (line[nread - 1] == '\n'){
					line[nread - 1] = '\0';
				}
				//printf("into small whileeeeeeeeee\n");
				//printf("line is: %s\n", line);
				
				map[count].text[lineCounterForDocuments] = (char *)malloc(((int)(nread))*sizeof(char));
				//printf("before strcpy\n");
				//fflush(stdout);
				if (map[count].text[lineCounterForDocuments] == NULL){
					printf("errno = %d\n", errno);
					printf("Current path is: %s\n", tempPathOfFile);
					printf("Current line is #%d '%s'\n", lineCounterForDocuments, line);
					perror("malloc at map[count].text[lineCounterForDocuments]");
					exit(EXIT_FAILURE);
				}
				strcpy(map[count].text[lineCounterForDocuments], line);
				token = strtok(line, " \t\n");
				//printf("before small small whileeeeeeeeeeeee\n");
				while (token != NULL){
					//printf("I am going to insert word: '%s'\n", token);
					totalWordsOfFiles++;
					length = (int)(strlen(token));
					insertWord(root, token, length, tempPathOfFile, lineCounterForDocuments/*, lineOffset*/);
					//printf("I did it successfully\n");
					token = strtok(NULL, " \t\n");
				}
				//lineOffset += nread;
				totalCharsOfFiles += nread;
				totalLinesOfFile++;
				lineCounterForDocuments++;
			}

			if (line != NULL){
				free(line);
				line = NULL;
			}
			//printf("after nread\n");
			
			if (fclose(fd) < 0){
				printf("errno = %d\n", errno);
				perror("fclose at worker");
				exit(EXIT_FAILURE);
			}
			/*
			if (close(fileDesc) < 0){
				printf("errno = %d\n", errno);
				perror("Close file of directory");
				exit(EXIT_FAILURE);
			}*/
			//printf("nread == %ld\n", nread);
			//lineOffset = 0;
			lineCounterForDocuments = 0;
			//lineCounterForDocuments = 0;
			docCounterInCatalog++;
			free(tempPathOfFile);
			count++;
		}
		//catalogCounter++;
		free(tempPath);
		rewinddir(dirPtr);
		if (closedir(dirPtr) == -1){
			printf("errno = %d\n", errno);
			perror("Cannot close directory");
			exit(EXIT_FAILURE);
		}
	}
	//printf("Trie is done!!!\n");
	fflush(stdout);
	//printf("count is: %d\n", count);


	//int *printToFile;

	char *query;
	char *arguments;
	//char *token;
	//count = 0;

	if ((fd_client_open = open(fifos.clientName, O_WRONLY)) < 0){
		printf("errno = %d\n", errno);
		printf("fifos.clientName = %s\n", fifos.clientName);
		perror("open fifos.clientName");
		exit(EXIT_FAILURE);
	}

	//printf("i am WORKER #%d and i am getting into while(1)\n", getpid());

	while (1){
		printf("waiting for order WORKER\n");
		//printf("------------------>>>>>>>>>>>>>>>>>new loop in while(1) at workerCode...\n");
		if ((readQueryLength = read(fd_server_open, &QueryLength, sizeof(QueryLength))) < 0){
			printf("errno = %d\n", errno);
			perror("read QueryLength");
			exit(EXIT_FAILURE);
		}
		printf("into worker.... QueryLength.lengthOfQuery == %d\n", QueryLength.lengthOfQuery);
		//QueryLength.lengthOfQuery = 20;
		//printf("into child #%d, QueryLength.lengthOfQuery = %d\n", getpid(), QueryLength.lengthOfQuery);
		query = (char *)malloc((QueryLength.lengthOfQuery + 1)*sizeof(char));
		if (query == NULL){
			printf("errno = %d\n", errno);
			perror("malloc of query");
			exit(EXIT_FAILURE);
		}
		//printf("QueryLength.lengthOfQuery = %d\n", QueryLength.lengthOfQuery);
		if ((readQuery = read(fd_server_open, query, QueryLength.lengthOfQuery)) < 0){
			printf("errno = %d\n", errno);
			perror("read query");
			exit(EXIT_FAILURE);
		}

		query[readQuery] = '\0';
		printf("query is: '%s'\n", query);
		token = strtok_r(query, " \t\n", &arguments);
		//printf("arguments of query are: '%s'\n", arguments);
		/*
		while (token != NULL){
			printf("token = '%s'\n", token);
			token = strtok(NULL, " \t\n");
		}*/
		if (strcmp(token, "/search") == 0){
			//sendMessageToAll(fifos, templine, numberOfWorkers);
			/*
			if (!ftime(&timer_msec)) {
		    	timestamp_msec_start = ((long long int) timer_msec.time)*1000ll + (long long int) timer_msec.millitm;
			}
			else{
				perror("ftime start");
				exit(EXIT_FAILURE);
			}
			*/

			templine = (char *)malloc(((int)(strlen(arguments) + 1))*sizeof(char));
			if (templine == NULL){
				printf("errno = %d\n", errno);
				perror("malloc templine at /search of worker");
				exit(EXIT_FAILURE);
			}
			
			strcpy(templine, arguments);
			tempLinePrint = (char *)malloc(((int)(strlen(arguments) + 1))*sizeof(char));
			if (tempLinePrint == NULL){
				printf("errno = %d\n", errno);
				perror("malloc tempLinePrint at /search of worker");
				exit(EXIT_FAILURE);
			}
			strcpy(tempLinePrint, arguments);


			numberOfWords = countWords(arguments);

			int **printToFile = (int **)malloc((numberOfWords)*sizeof(int *));
			if (printToFile == NULL){
				printf("errno = %d\n", errno);
				perror("malloc printToFile");
				exit(EXIT_FAILURE);
			}
			/*
			for (int i = 0; i < (numberOfWords - 2); i++){
				printToFile[i] = (int *)malloc(tempCount2*sizeof(int));
				if (printToFile[i] == NULL){
					printf("errno = %d\n", errno);
					printf("printToFile[%d] == NULL\n", i);
					perror("malloc printToFile[i]");
					exit(EXIT_FAILURE);
				}
			}
			*/

			//printf("numberOfWords = %d\n", numberOfWords);
			//printf("arguments = '%s' and templine = '%s'\n", arguments, templine);
			token = strtok(templine, " \t\n");
			for (i = 0; i < (numberOfWords); i++){	//-2 is for -d and deadline
				printToFile[i] = pinMapSearch(search(root, token, (int)(strlen(token))), map, tempCount2/*, &exitCounter*/);
				token = strtok(NULL, " \t\n");
			}

			/*
			if (!ftime(&timer_msec)) {
				timestamp_msec_end = ((long long int) timer_msec.time)*1000ll + (long long int) timer_msec.millitm;
			}
			else{
				perror("ftime end");
				exit(EXIT_FAILURE);
			}
			countLinesToSendForSearch = 0;
			for (i = 0; i < tempCount2; i++){
				for (j = 0; j < map[i].numLines; j++){
					//printf(".map[%d].searchQuery[%d] = %d\n", i, j, map[i].searchQuery[j]);
					if (map[i].searchQuery[j] == 1){
						countLinesToSendForSearch++;
						//printf("yeah\n");
					}
				}
			}
			*/

			/*
			timestamp_msec_final = timestamp_msec_end - timestamp_msec_start;

			if (timestamp_msec_final > ((long long int)getDeadline(templine))){
				startingSearch.intoDeadline = -1;
				startingSearch.numberOfResults = countLinesToSendForSearch;
			}
			else{
				startingSearch.intoDeadline = 1;
				startingSearch.numberOfResults = countLinesToSendForSearch;
				//printf("tokeeeeeeeen = '%s'\n", token);
				
				char buff[20];
				struct tm *sTm;
				time_t now = time (0);
				sTm = gmtime (&now);
				strftime (buff, sizeof(buff), "%Y-%m-%d %H:%M:%S", sTm);
				token =strtok(tempLinePrint, " \t\n");
				for (i = 0; i < (numberOfWords - 2); i++){
					fprintf(fd_log_file, "%s : /search : %s", buff, token);
					for (j = 0; j < tempCount2; j++){
						if (printToFile[i][j] == 1){
							fprintf(fd_log_file, " : %s", map[j].filePath);
						}
					}
					fprintf(fd_log_file, " \n");
					exitCounter++;
					token = strtok(NULL, " \t\n");
				}
			}
			*/

			if ((nwrite = write(fd_client_open, &startingSearch, sizeof(startingSearch))) < 0){
				printf("errno = %d\n", errno);
				printf("I am worker #%d\n", getpid());
				perror("write to jobExecutor startingSearch");
				exit(EXIT_FAILURE);
			}

			//if (startingSearch.intoDeadline == 1){
				//printf("before first for\n");
				//fflush(stdout);
				for (i = 0; i < tempCount2; i++){
					//printf("i into worker is %d\n", i);
					//fflush(stdout);
					for (j = 0; j < map[i].numLines; j++){
						//printf("j into worker is %d\n", j);
						if (map[i].searchQuery[j] == 1){
							searchInfo.sizeOfPath = (int)(strlen(map[i].filePath));
							searchInfo.numberOfLine = j;
							searchInfo.sizeOfLine = (int)(strlen(map[i].text[j]));

							if ((nwrite = write(fd_client_open, &searchInfo, sizeof(searchInfo))) < 0){
								printf("errno = %d\n", errno);
								printf("I am worker #%d and (i,j) is (%d,%d)\n", getpid(), i, j);
								perror("Write to jobExecutor searchInfo");
								exit(EXIT_FAILURE);
							}
							searchInfo.sizeOfPath = 0;
							searchInfo.numberOfLine = 0;
							searchInfo.sizeOfLine = 0;

							if ((nwrite = write(fd_client_open, map[i].filePath, (int)(strlen(map[i].filePath)))) < 0){
								printf("errno = %d\n", errno);
								printf("I am worker #%d and (i,j) is (%d,%d)\n", getpid(), i, j);
								perror("Write to jobExecutor map[i].filePath");
								exit(EXIT_FAILURE);
							}

							if ((nwrite = write(fd_client_open, map[i].text[j], (int)(strlen(map[i].text[j])))) < 0){
								printf("errno = %d\n", errno);
								printf("I am worker #%d and (i,j) is (%d,%d)\n", getpid(), i, j);
								perror("Write to jobExecutor map[i].text[j]");
								exit(EXIT_FAILURE);
							}
						}
					}
					//printf("skataaaaa\n");
				}
			//}
			if (templine != NULL){
				free(templine);
				templine = NULL;
			}
			unpinMapSearch(map, tempCount2);

			for (i = 0; i < (numberOfWords - 2); i++){
				if (printToFile[i] != NULL){
					free(printToFile[i]);
					printToFile[i] = NULL;
				}
			}
			if (printToFile != NULL){
				free(printToFile);
				printToFile = NULL;
			}

			if (tempLinePrint != NULL){
				free(tempLinePrint);
				tempLinePrint = NULL;
			} 

			startingSearch.intoDeadline = 1;
			startingSearch.numberOfResults = 0;
		}
		
		else if (strcmp(token, "/exit") == 0){
			if ((nwrite = write(fd_client_open, &exitCounter, sizeof(exitCounter))) < 0){
				printf("errno = %d\n", errno);
				printf("I am worker #%d\n", getpid());
				perror("write to jobExecutor exit strings");
				exit(EXIT_FAILURE);
			}
			break;
		}
		

		//query[0] = '\0';
		if (query != NULL){
			free(query);
			query = NULL;
		}

	}


	if (query != NULL){
		free(query);
		query = NULL;
	}

	if (logFilePath != NULL){
		free(logFilePath);
		logFilePath = NULL;
	}
	for (int i = 0; i < argStruct.numOfArguments; i++){
		if (Directories[i] != NULL){
			free(Directories[i]);
			Directories[i] = NULL;
		}
	}
	if (Directories != NULL){
		free(Directories);
		Directories = NULL;
	}
	for (int i = 0; i < tempCount2; i++){
		if (map[i].filePath != NULL){
			free(map[i].filePath);
			map[i].filePath = NULL;
		}
		if (map[i].searchQuery != NULL){
			free(map[i].searchQuery);
			map[i].searchQuery = NULL;
		}
		for (int j = 0; j < map[i].numLines; j++){
			if (map[i].text[j] != NULL){
				free(map[i].text[j]);
				map[i].text[j] = NULL;
			}
		}
		if (map[i].text != NULL){
			free(map[i].text);
			map[i].text = NULL;
		}
	}
	if (map != NULL){
		free(map);
		map = NULL;
	}

	
	if (fclose(fd_log_file) == EOF){
		printf("errno = %d\n", errno);
		perror("fclose worker");
		exit(EXIT_FAILURE);
	}
	if(close(fd_client_open) < 0){
		printf("errno = %d\n", errno);
		perror("close worker fd_client_open");
		exit(EXIT_FAILURE);
	}
	if(close(fd_server_open) < 0){
		printf("errno = %d\n", errno);
		perror("close worker fd_server_open");
		exit(EXIT_FAILURE);
	}


	if (unlink(fifos.clientName) < 0){
		printf("errno = %d\n", errno);
		perror("unlink fifos.clientName");
		exit(EXIT_FAILURE);
	}

	if (unlink(fifos.serverName) < 0){
		printf("errno = %d\n", errno);
		perror("unlink fifos.serverName");
		exit(EXIT_FAILURE);
	}
	
	if (fifos.clientName != NULL){
		free(fifos.clientName);
		fifos.clientName = NULL;
	}
	if (fifos.serverName != NULL){
		free(fifos.serverName);
		fifos.serverName = NULL;
	}


	deleteTrie(root);

	exit(EXIT_SUCCESS);
}