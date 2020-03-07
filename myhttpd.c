#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/wait.h> 		/* sockets */
#include <sys/types.h> 		/* sockets */
#include <sys/socket.h> 	/* sockets */
#include <netinet/in.h> 	/* internet sockets */
#include <netdb.h> 			/* gethostbyaddr */
#include <unistd.h> 		/* fork */
#include <ctype.h> 			/* toupper */
#include <signal.h> 		/* signal */
#include <libgen.h>
#include "generalFunctionsServer.h"
#include "threadsFunctions.h"
#include "variables.h"
#include "generalFunctions.h"


int main(int argc, char const *argv[]){

	char bufCommand[1600], *token, *argument;
	int servingPort, commandPort, numOfThreads, sockCommand, sock, newsock, newsockCommand, i, nrd;
	//long int counter;
	struct sockaddr_in server, client, serverCommand;
	int err, exit_flag = 0;
	socklen_t clientlen;
//	char *bname;
//	char *headerReturn;
	//char headerReturn[1000];
	//char date[35];
	//struct tm *sTm;
	//time_t now;
	//FILE *fp;

	time_t start_time, current_time, difference;
	start_time = time(NULL);

	fd_set active_fd_set, read_fd_set;


	checkParameters(argc, argv);

	servingPort = atoi(argv[2]);
	commandPort = atoi(argv[4]);
	numOfThreads = atoi(argv[6]);
	root_dir = (char *)argv[8];


	pthread_t consumers[numOfThreads];
	initialize(&pool);
	pthread_mutex_init(&mtx, 0);
	pthread_mutex_init(&count_mtx, 0);
	pthread_cond_init(& cond_nonempty, 0);
	pthread_cond_init(& cond_nonfull, 0);
	for (i = 0; i < numOfThreads; i++){
		if ((err = pthread_create(&consumers[i], 0, pthread_consumer, 0))){
			perror2("pthread_create", err);
			exit(EXIT_FAILURE);
		}
	}

	printf(">>> I am main thread with id: %ld\n", pthread_self());

	/*create socket*/
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		perror_exit("socket");
	}

	if ((sockCommand = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		perror_exit("socket Command");
	}


	struct sockaddr *serverptr = (struct sockaddr *)&server;
	struct sockaddr *serverptrCommand = (struct sockaddr *)&serverCommand;
	struct sockaddr *clientptr = (struct sockaddr *)&client;

	server.sin_family = AF_INET; 				/* Internet domain */
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	server.sin_port = htons(servingPort);		/* The given port */

	serverCommand.sin_family = AF_INET; 				/* Internet domain */
	serverCommand.sin_addr.s_addr = htonl(INADDR_ANY);
	serverCommand.sin_port = htons(commandPort);		/* The given port */

	/* Bind socket to address */
	if (bind(sock, serverptr, sizeof(server)) < 0){
		perror_exit("bind server");
	}

	/* Bind socketCommand to address */
	if (bind(sockCommand, serverptrCommand, sizeof(serverCommand)) < 0){
		perror_exit("bind server command");
	}

	/* Listen for connections */
	if (printf("%d\n", listen(sock, SOMAXCONN)) < 0){
		perror_exit("listen sock");
	}

	/* Listen for connections */
	if (printf("%d\n", listen(sockCommand, SOMAXCONN)) < 0){
		perror_exit("listen sockCommand");
	}

	/* Initialize the set of active sockets. */
	FD_ZERO (&active_fd_set);
	FD_SET (sock, &active_fd_set);
	FD_SET (sockCommand, &active_fd_set);
	int max_fd;
	if (sock > sockCommand){
		max_fd = sock + 1;
	}
	else {
		max_fd = sockCommand + 1;
	}


	while(1){


		read_fd_set = active_fd_set;
		if (select (max_fd, &read_fd_set, NULL, NULL, NULL) < 0){
			perror_exit("select");
        }
        for (i = 0; i < max_fd; i++){
        	printf("i is: %d\n", i);
        	//printf("FD_SETSIZE is: %d\n", max_fd);
			if (FD_ISSET (i, &read_fd_set)){

				if (i == sock){
					/* accept connection */

					if ((newsock = accept(sock, clientptr, &clientlen)) < 0){
						perror_exit("accept");
					}

					printf("Accepted connection\n");
					printf("I am thread %ld and I will put in pool %d\n", pthread_self(), newsock);			
					place(&pool , newsock);
					//buf[0] = '\0';
				}
				else if (i == sockCommand){
					if ((newsockCommand = accept(sockCommand, clientptr, &clientlen)) < 0){
						perror_exit("accept");
					}

					printf("Accepted connection 2\n");
					if ((nrd = read(newsockCommand, bufCommand, sizeof(bufCommand))) > 0){
						//printf("--------------------------\n");
						//printf("\n%s\n", bufCommand);
						token = strtok_r(bufCommand, " \t\n", &argument);
						if ((strcmp(token, "GET") == 0) || (strcmp(token, "get") == 0)){
							token = strtok(argument, " \t\n");
							if (strcmp(token, "/STATS") == 0){
								current_time = time(NULL);
								difference = current_time - start_time;
								pthread_mutex_lock(&count_mtx);
								printf("Server up for %ld:%ld.%ld, served %d pages, %ld bytes\n", (difference/3600), (difference/60), (difference%60), servedPages, servedBytes);
								pthread_mutex_unlock(&count_mtx);
							}
							else if (strcmp(token, "/SHUTDOWN") == 0){
								printf("--------->%s\n", token);
								exit_flag = 1;
								place(&pool , -1);
								//pool->count = -10
								pthread_cond_broadcast(&cond_nonempty);
								break;
							}
						}
						else {
							fprintf(stderr, "Unknown command of http protocol!\n");
						}
					}
					//bufCommand[0] = '\0';
					//printf("skataaaaaa\n");
				}
			}
        }
        if (exit_flag == 1){
        	break;
        }	
	}



	for (i = 0; i < numOfThreads; i++){
		printf("ok\n");
		if ((err = pthread_join(consumers[i], NULL))){
			perror2("pthread_join", err);
			exit(EXIT_FAILURE);
		}
	}
	printf("All threads terminated\n");
	if ((err = pthread_mutex_destroy(&mtx))){
		perror2("pthread_mutex_destroy (mtx)", err);
		exit(EXIT_FAILURE);
	}
	if ((err = pthread_mutex_destroy(&count_mtx))){
		perror2("pthread_mutex_destroy (count_mtx)", err);
		exit(EXIT_FAILURE);
	}
	if (((err = pthread_cond_destroy(&cond_nonempty)))){
		perror2("pthread_cond_destroy (cond_nonempty)", err);
		exit(EXIT_FAILURE);
	}
	if ((err = pthread_cond_destroy(&cond_nonfull))){
		perror2("pthread_cond_destroy (cond_nonfull)", err);
		exit(EXIT_FAILURE);
	}

	pthread_exit(NULL);
}