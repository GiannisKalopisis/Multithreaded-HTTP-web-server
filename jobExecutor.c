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
#include <signal.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/wait.h> 		/* sockets */
#include <sys/types.h> 		/* sockets */
#include <sys/socket.h> 	/* sockets */
#include <netinet/in.h> 	/* internet sockets */
#include <netdb.h> 			/* gethostbyaddr */
#include <libgen.h>
#include "generalFunctionsJE.h"
#include "structs.h"




#define PERMS 0666
#define SERVER_FIFO_NAME "fifo_server_"
#define CLIENT_FIFO_NAME "fifo_client_"
#define READ 0
#define WRITE 1

#define COMMANDPORT 8899


char **pathArray;
FifoNames *fifos;
int globalNumberOfWorkers;


int main (int argc, char const *argv[]){

	int numberOfWorkers, docfileDesc, numberOfLines = 0, numberOfRowsInWorkers, mod = 0, j = 0, lengthOfWorkersProgram = 0, lengthOfPath, counter = 0, readPath;
	int lengthOfpid, i, readMIN, tempReadPath, tempReadMIN, readMAX, timeSearch, tempReadLine, answerWorkers = 0;
	char **args, *workerProgram = "./workerCode", *line = NULL, *token, *arguments, *temp, *filePathForMaxMin, *tempFilePathForMaxMin, *finalPathMaxMin = NULL, *tempPath = NULL, *tempSearchLine = NULL;	
	pid_t pid, tempPid;
	size_t name_max, len = 0;
	ssize_t nread;
	int nwrite = 0, maxDirLength = 0, readWC = 0, fd_read = 0, k = 0, wordTimesCountMaxMin = 0, onlyOneWorker, searchingInfo = 0;
	int numberOfResults = 0, numberO_R, flag = 0, exitCounter = 0, totalExitCounter = 0;//, sizeOfPath = 0, numberOfLine = 0, sizeOf;
	passingArgumentsStruct passingArguments;
	sendStruct argStruct;
	passingQueryLength QueryLength;
	wc_Struct wcStruct;
	MaxMinSendStruct maxMinSendStruct;
	startingSearchStruct startingSearch;
	searchInfoStruct searchInfo;




	lengthOfWorkersProgram = (int)(strlen(workerProgram));

	checkParameters(argc, argv);
	numberOfWorkers = 4; 
	int fd = 0;


	if ((docfileDesc = open(argv[2], O_RDONLY, PERMS)) == -1){
		printf("errno = %d\n", errno);
		perror("Opening docfile");
		exit(EXIT_FAILURE);
	}

	numberOfLines = countLines(docfileDesc);
	if (numberOfWorkers > numberOfLines){
		numberOfWorkers = numberOfLines;
	}


	//int fd_server_write;
	//int fd_client_read;

	pathArray = getPathsFromFile(docfileDesc, numberOfLines, &maxDirLength);
	//printf("maxDirLength: %d\n", maxDirLength);
	//for (int k = 0; k < numberOfLines; k++){
	//	printf("'%s'\n", pathArray[k]);
	//}
	char Arg[maxDirLength + 1];
	for (i = 0; i < (maxDirLength + 1); ++i){
		Arg[i] = '/';
	}
	

	numberOfRowsInWorkers = (numberOfLines / numberOfWorkers);
	mod = (numberOfLines % numberOfWorkers);

	fifos = (FifoNames *) malloc(numberOfWorkers*sizeof(FifoNames));
	if (fifos == NULL){
		printf("errno = %d\n", errno);
		perror("malloc");
		exit(EXIT_FAILURE);
	}
	globalNumberOfWorkers = numberOfWorkers;


	for (i = 0; i < numberOfWorkers; i++){
		
		if ((i == (numberOfWorkers - 1)) && (mod != 0)){
			//printf("numberOfLines = %d\n", numberOfLines);
			//printf("counter = %d\n", counter);
			numberOfRowsInWorkers = numberOfLines - counter; 
		}
		//malloc the array of pointers
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
		tempPid = getpid();
		char *tempToArray = toArray(tempPid);
		strcpy(args[1], tempToArray);
		//last argument must be NULL
		args[2] = NULL;

		if ((pid = fork()) == -1 ){
			printf("errno = %d\n", errno);
			perror("fork call");
			exit(EXIT_FAILURE);
		}
		if (pid == 0){
			execvp(args[0],args);
			printf("errno = %d\n", errno);
			perror("execvp");
			exit(EXIT_FAILURE);
		}

		fifos[i].serverName = (char *) malloc(((int)((int)strlen(SERVER_FIFO_NAME) + digitCounterFunction(getpid()) + digitCounterFunction(pid) + 2))*sizeof(char));		//+1 is for '\0'
		if (fifos[i].serverName == NULL){
			perror("malloc at fifos[i].serverName 1");
			exit(EXIT_FAILURE);
		}
		fifos[i].serverName = make_fifo_name(getpid(), fifos[i].serverName, fifos[i].serverName, (int)(strlen(SERVER_FIFO_NAME)), 0);
		if (fifos[i].serverName == NULL){
			perror("make_fifo_name server");
			exit(EXIT_FAILURE);
		}
		temp = (char *) malloc(((int)((int)(strlen(fifos[i].serverName)) + 1))*sizeof(char));		//+1 is for '\0'
		if (temp == NULL){
			printf("errno = %d\n", errno);
			perror("malloc at temp");
			exit(EXIT_FAILURE);
		}
		strcpy(temp, fifos[i].serverName);
		fifos[i].serverName = make_fifo_name(pid, temp, fifos[i].serverName, (int)(strlen(fifos[i].serverName)), 2);
		if (fifos[i].serverName == NULL){
			perror("make_fifo_name server");
			exit(EXIT_FAILURE);
		}

		fifos[i].clientName = (char *) malloc(((int)((int)(strlen(CLIENT_FIFO_NAME)) + digitCounterFunction(pid) + 1))*sizeof(char));		//+1 is for '\0'
		if (fifos[i].clientName == NULL){
			perror("malloc");
			exit(EXIT_FAILURE);
		}
		fifos[i].clientName = make_fifo_name(pid, fifos[i].serverName, fifos[i].clientName, (int)(strlen(CLIENT_FIFO_NAME)), 1);
		if (fifos[i].clientName == NULL){
			perror("make_fifo_name client");
			exit(EXIT_FAILURE);
		}

		
		if (mkfifo(fifos[i].serverName, PERMS) == -1){
			//printf("---->\n");
			//printf("%s\n", fifos[i].clientName);
			if (errno != EEXIST){
				printf("errno = %d\n", errno);
				perror("receiver: mkfifo (server)");
				printf("Exiting kid %d\n", getpid());
				exit(EXIT_FAILURE);
			}
		}
		
		if (mkfifo(fifos[i].clientName, PERMS) == -1){
			//printf("'@'%s'@'\n", fifos[i].serverName);
			if (errno != EEXIST){
				printf("errno = %d\n", errno);
				perror("receiver: mkfifo (client)");
				printf("Exiting kid %d\n", getpid());
				exit(EXIT_FAILURE);
			}
		}

		if ((fifos[i].fd_server_client = open(fifos[i].serverName, O_WRONLY)) < 0){
			printf("errno = %d\n", errno);
			perror("open fifos[i].serverName");
			exit(EXIT_FAILURE);
		}

		if ((fifos[i].fd_client_server = open(fifos[i].clientName, O_RDONLY)) < 0){
			printf("errno = %d\n", errno);
			printf("fifos[%d].clientName = %s\n", i, fifos[i].clientName);
			perror("open fifos[i].clientName");
			exit(EXIT_FAILURE);
		}

		argStruct.numOfArguments = numberOfRowsInWorkers;
		argStruct.maxDirSize = maxDirLength + 1;
		if ((nwrite = write(fifos[i].fd_server_client, &argStruct, sizeof(argStruct))) < 0){
			printf("errno = %d\n", errno);
			printf("i : %d\n", i);
			perror("write argStruct on fifos[i].serverName");
			exit(EXIT_FAILURE);
		}
		//printf("i am jobExecutor and i just wrote size of argStructin fifo[%d]\n", i);
		for (j = 0; j < numberOfRowsInWorkers; j++){
			//printf("--------------\n");
			//strcpy(Arg, pathArray[counter]);
			for (k = 0; k < (int)(strlen(pathArray[counter])); k++)
			{
				Arg[k] = pathArray[counter][k];
			}
			//printf("sending arguments: '%s'\n", Arg);
			if ((nwrite = write(fifos[i].fd_server_client, Arg, argStruct.maxDirSize)) < 0){
				printf("errno = %d\n", errno);
				printf("i : %d\n", i);
				perror("write numberOfRowsInWorkers on fifos[i].serverName");
				exit(EXIT_FAILURE);
			}
			//printf("i am jobExecutor and i just wrote Arg[%d] into fifo[%d]\n", j, i);
			//printf("--->just send: '%s'\n", Arg);
			for (k = 0; k < maxDirLength + 1; k++){
				Arg[k] = '/';
			}
			counter++;
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

		if (tempToArray != NULL){
			free(tempToArray);
			tempToArray = NULL;
		}
		
		if (temp != NULL){
			free(temp);
			temp = NULL;
		}
	}

	socklen_t clientlen = 0;
	int sockCommand;

	struct sockaddr_in serverCommand, client;
	struct sockaddr *clientptr = (struct sockaddr *)&client;
	struct sockaddr *serverptrCommand = (struct sockaddr *)&serverCommand;

	serverCommand.sin_family = AF_INET; 				/* Internet domain */
	serverCommand.sin_addr.s_addr = htonl(INADDR_ANY);
	serverCommand.sin_port = htons(COMMANDPORT);		/* The given port */

	/*create socket*/
	if ((sockCommand = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		perror_exit("socket Command");
	}

	int enableSocketADDR = 1;
	if (setsockopt(sockCommand, SOL_SOCKET, SO_REUSEADDR, &enableSocketADDR, sizeof(int)) < 0){
		perror_exit("setsockopt(SO_REUSEADDR)");
	}

	/* Bind socketCommand to address */
	if (bind(sockCommand, serverptrCommand, sizeof(serverCommand)) < 0){
		perror_exit("bind server command");
	}

	if (listen(sockCommand, SOMAXCONN) < 0){
		perror_exit("listen sockCommand");
	}



	int totalBytes = 0, totalWords = 0, totalLines = 0;
	//printCommands();
	//char templine[2000];
	char **writeBuffer = NULL;
	int writeBytesCounter = 0, newsockCommand, nrd;
	char strNumber[10];

	char workerAnswer[1000];
	//char writeBuffer[3000];
	char *templine;
	int templineNum = 0;

		
		while (1){

			clientlen = 0;
			clientptr = (struct sockaddr *)&client;

			//printf("templine is: '%s'\n", templine);

			templine = (char *)malloc(20*sizeof(char));
			if (templine == NULL){
				perror_exit("malloc");
			}
			//memset(templine, '\0', sizeof(templine));

			if ((newsockCommand = accept(sockCommand, clientptr, &clientlen)) < 0){
				perror_exit("accept");
			}

			printf("------------------>>>>>>>>>>>>>>>>>new loop in while(1) at jobExecutor...\n");
			printf("sizeof(templine) before read == %lu\n", sizeof(templine));

			if ((nrd = read(newsockCommand, templine, 20*sizeof(char))) < 0){
				perror_exit("read");
			}

			for (int i = 0; i < 20; i++){
				if (templine[i] == '!'){

					templine[i] = '\0';
				}
			}
			templineNum = atoi(templine);
			printf("templineNum == %d\n", templineNum);

			//memset(templine, '\0', sizeof(templine));
			char *temp2 = (char *)malloc((templineNum + 1)*sizeof(char));
			if (temp2 == NULL){
				perror_exit("malloc");
			}
			memset(temp2, '\0', (templineNum + 1));
			printf("sizeof(temp2) == %ld\n", sizeof(temp2));
			if ((nrd = read(newsockCommand, temp2, templineNum)) < 0){
				perror_exit("read");
			}
			printf("temp2 into JE == '%s'\n", temp2);
			printf("nrd == %d\n", nrd);
			//temp2[nrd+1] = '\0';
			printf("after temp2 into JE == '%s'\n", temp2);
			printf("strlen(templine) == %ld\n", strlen(temp2));


			QueryLength.lengthOfQuery = templineNum;

			char *temp3 = (char *)malloc((templineNum + 1)*sizeof(char));
			if (temp3 == NULL){
				perror_exit("malloc");
			}
			memset(temp3, '\0', (templineNum + 1));
			strcpy(temp3, temp2);
			//printf("%s\n", );

			printf("QueryLength.lengthOfQuery == %d\n", QueryLength.lengthOfQuery);

			token = strtok_r(temp2, " \t\n", &arguments);
			if (strcmp(token, "/search") == 0){
				//timeSearch = searchArgumentsCheck(arguments); 
				//if (timeSearch > 0){


					printf("numberOfWorkers == %d\n", numberOfWorkers);
					for (k = 0; k < numberOfWorkers; k++){
						printf("i just send from JE to worker QueryLength.lengthOfQuery == %d, arguments == '%s'\n", QueryLength.lengthOfQuery, arguments);
						if ((nwrite = write(fifos[k].fd_server_client, &QueryLength, sizeof(QueryLength))) < 0){
							printf("errno = %d\n", errno);
							printf("k : %d\n", k);
							perror("send QueryLength on search");
							exit(EXIT_FAILURE);
						}
						printf("after first write to workers \n");
						if ((nwrite = write(fifos[k].fd_server_client, temp3, QueryLength.lengthOfQuery)) < 0){
							printf("errno = %d\n", errno);
							printf("k : %d\n", k);
							perror("send templine on search");
							exit(EXIT_FAILURE);
						}
					}

					printf("numberOfWorkers == %d\n", numberOfWorkers);
					int counterForWriteBuffer = 0;
					for (k = 0; k < numberOfWorkers; k++){

						if ((numberO_R = read(fifos[k].fd_client_server, &startingSearch, sizeof(startingSearchStruct))) < 0){
							printf("errno = %d\n", errno);
							printf("At worker #%d\n", k);
							perror("read numberOfResults");
							exit(EXIT_FAILURE);
						}
						//printf("into startingSearch.intoDeadline\n");
						//fflush(stdout);


						answerWorkers++;
						for (j = 0; j < startingSearch.numberOfResults; j++){
							//printf("j = %d\n", j);
							//printf("k = %d\n", k);
							if ((searchingInfo = read(fifos[k].fd_client_server, &searchInfo, sizeof(searchInfo))) < 0){
								printf("errno = %d\n", errno);
								printf("j = %d\n", j);
								perror("read searchingInfo");
								exit(EXIT_FAILURE);
							}

							if (tempPath != NULL){
								free(tempPath);
								tempPath = NULL;
							}
							tempPath = (char *)malloc((searchInfo.sizeOfPath + 1)*sizeof(char));
							if (tempPath == NULL){
								printf("errno = %d\n", errno);
								perror("malloc at tempPath of /search");
								exit(EXIT_FAILURE);
							}
							if ((tempReadPath = read(fifos[k].fd_client_server, tempPath, searchInfo.sizeOfPath)) < 0){
								printf("errno = %d\n", errno);
								perror("read path of file into /search");
								exit(EXIT_FAILURE);
							}
							//tempPath[tempReadPath] = '\0';
								
							tempSearchLine = (char *)malloc((searchInfo.sizeOfLine + 1)*sizeof(char));
							if (tempSearchLine == NULL){
								printf("errno = %d\n", errno);
								perror("malloc at tempSearchLine of /search");
								exit(EXIT_FAILURE);
							}
							if ((tempReadLine = read(fifos[k].fd_client_server, tempSearchLine, searchInfo.sizeOfLine)) < 0){
								printf("errno = %d\n", errno);
								perror("read line of file into /search");
								exit(EXIT_FAILURE);
							}

							printf("i took 1 answer\n");
							printf("%s\n", tempPath);

							memset(workerAnswer, '\0', sizeof(workerAnswer));
							//fflush(stdout);
							sprintf(workerAnswer, "%s %d %s\n", tempPath, searchInfo.numberOfLine, tempSearchLine);
							printf("workerAnswer == '%s'\n", workerAnswer);
							fflush(stdout);
							/*
							fflush(stdout);
							writeBuffer = (char **)realloc(writeBuffer, (counterForWriteBuffer + 1)*sizeof(*writeBuffer));
							if (writeBuffer == NULL){
								perror_exit("realloc");
							}

							writeBuffer[j] = (char *)malloc((strlen(workerAnswer) + 1)*sizeof(char));
							if (writeBuffer[j] == NULL){
								perror_exit("malloc");	
							}
							printf("workerAnswer == '%s'\n", workerAnswer);
							strcpy(writeBuffer[j], workerAnswer);

							counterForWriteBuffer++;
							*/


							
							
							if (write(newsockCommand, workerAnswer, sizeof(workerAnswer)) < 0){
								perror_exit("write");
							}
							
							



							//strcat(writeBuffer, workerAnswer);


							fflush(stdout);
								
							if (tempSearchLine != NULL){
								free(tempSearchLine);
								tempSearchLine = NULL;
							}
							if (tempPath != NULL){
								free(tempPath);
								tempPath = NULL;
							}
						}
					}

					memset(workerAnswer, '#', sizeof(workerAnswer));
					if (write(newsockCommand, workerAnswer, sizeof(workerAnswer)) < 0){
						perror_exit("write");
					}

					/*
					memset(strNumber, '\0', sizeof(strNumber));
					printf("sizeof(strNumber) == %lu\n", sizeof(strNumber));
					sprintf(strNumber, "%d", counterForWriteBuffer);

					for (int i = 0; i < counterForWriteBuffer; ++i)
					{
						printf("%s\n", writeBuffer[i]);
					}

					if (write(newsockCommand, strNumber, sizeof(strNumber)) < 0){
						perror_exit("write");
					}
					for (int i = 0; i < counterForWriteBuffer; i++){
						
						memset(strNumber, '\0', sizeof(strNumber));
						sprintf(strNumber, "%ld", strlen(writeBuffer[i]));
						printf("writing back to Crawler\n");
						if (write(newsockCommand, strNumber, strlen(strNumber)) < 0){
							perror_exit("write");
						}

						//for (int l = 0; l < strlen(writeBuffer[i]); l++){
						if (write(newsockCommand, writeBuffer[i], sizeof(writeBuffer[i])) < 0){
							perror_exit("write");
						}
						//}
					}


					for (int i = 0; i < j; i++){
						if (writeBuffer[i] != NULL){
							free(writeBuffer[i]);
							writeBuffer[i] = NULL;
						}
					}
					if (writeBuffer != NULL){
						free(writeBuffer);
						writeBuffer = NULL;
					}
					*/
					free(temp3);
					temp3 = NULL;
					free(temp2);
					temp2 = NULL;

					answerWorkers = 0;
					
				//}
				//else if (timeSearch == -2){
				//	printf("Not enough arguments for /search. You need at least 2 arguments (1 word to search and \"/search\")\n\n");
				//}

			}
			else if (strcmp(token, "/exit") == 0){

				free(temp3);
				temp3 = NULL;
				free(temp2);
				temp2 = NULL;

				for (k = 0; k < numberOfWorkers; k++){
					if ((nwrite = write(fifos[k].fd_server_client, &QueryLength, sizeof(QueryLength))) < 0){
						printf("errno = %d\n", errno);
						printf("i : %d\n", i);
						perror("write argStruct on fifos[i].serverName");
						exit(EXIT_FAILURE);
					}
					if ((nwrite = write(fifos[k].fd_server_client, templine, QueryLength.lengthOfQuery)) < 0){
						printf("errno = %d\n", errno);
						printf("i : %d\n", i);
						perror("write argStruct on fifos[i].serverName");
						exit(EXIT_FAILURE);
					}
				}
				for (k = 0; k < numberOfWorkers; k++){
					if ((readWC = read(fifos[k].fd_client_server, &exitCounter, sizeof(exitCounter))) < 0){
						printf("errno = %d\n", errno);
						perror("read exit results");
						exit(EXIT_FAILURE);
					}
					totalExitCounter += exitCounter;
				}
				printf("I am exiting jobExecutor with #%d strings.\n", totalExitCounter);
				break;
			}
			
			else {
				printf("ERROR: Wrong format of the command %s!!!\n", line);
				printCommands();
			}

			if (templine != NULL){
				free(templine);
				templine = NULL;
			}
		}


	close(newsockCommand);

	for (i = 0; i < numberOfWorkers; i++){
		if(close(fifos[i].fd_server_client) < 0){
			printf("errno = %d\n", errno);
			printf("i = %d\n", i);
			perror("close jobExecutor fifos[i].fd_server_client");
			exit(EXIT_FAILURE);
		}
		if(close(fifos[i].fd_client_server) < 0){
			printf("errno = %d\n", errno);
			printf("i = %d\n", i);
			perror("close jobExecutor fifos[i].fd_client_server");
			exit(EXIT_FAILURE);
		}
		
		if (fifos[i].serverName != NULL){
			free(fifos[i].serverName);
			fifos[i].serverName = NULL;
		}
		if (fifos[i].clientName != NULL){
			free(fifos[i].clientName);
			fifos[i].clientName = NULL;
		}		
	}

	if (fifos != NULL){
		free(fifos);
		fifos = NULL;
	}


	for (int i = 0; i < numberOfLines; i++){
	 	if (pathArray[i] != NULL){
	 		free(pathArray[i]);
	 		pathArray[i] = NULL;
	 	}
	}
	if (pathArray != NULL){
		free(pathArray);
		pathArray = NULL;
	}
	/*
	if (close(docfileDesc) < 0){
		printf("errno = %d\n", errno);
		perror("close");
		exit(EXIT_FAILURE);
	}*/
	


	free(line);
	exit(EXIT_SUCCESS);
}