#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <fcntl.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/wait.h> 		/* sockets */
#include <sys/types.h> 		/* sockets */
#include <sys/socket.h> 	/* sockets */
#include <netinet/in.h> 	/* internet sockets */
#include <netdb.h> 			/* gethostbyaddr */
#include <unistd.h> 		/* fork */
#include <ctype.h> 			/* toupper */
#include <signal.h> 		/* signal */
#include <libgen.h>
#include "generalFunctionsCrawler.h"
#include "structCrawler.h"
#include "threadsFunctions.h"
#include "variables.h"
#include "generalFunctions.h"





int main(int argc, char const *argv[]){
	

	int command_port, sockCommand, newsockCommand, sockSearch, nrd, first_time_search = 0;
	char bufCommand[2000], *token, *argument, *fileName = NULL, **searchWords = NULL, *argument2;



	END_ALL = 0; downloadedPages = 0; downloadedBytes = 0; serving_threads = 0; globalCounter = 0;

	Exist_Head existHead2;

	int i=0, err;
	//char hostname [50];
	socklen_t clientlen = 0;
	struct in_addr  **addr_list;

	struct sockaddr_in serverCommand, client;
	struct sockaddr *clientptr = (struct sockaddr *)&client;
	//struct hostent *rem;

	time_t start_time, current_time, difference;
	start_time = time(NULL);

	checkParametersCrawler(argc, argv);

	port = atoi(argv[4]);
	command_port = atoi(argv[6]);
	num_of_threads = atoi(argv[8]);

	save_dir = makeDir(argv[10]);

	initURLQueueHead(&urlQueue);
	initExistQueueHead(&existHead);
	initExistQueueHead(&existHead2);
	
	push(&urlQueue, getURL((char *)argv[11]), &existHead);


	//Resolving IP
	if ((mymachine = gethostbyname(argv[2])) == NULL){
		herror("gethostbyname"); 
		exit(EXIT_FAILURE);
	}
	else{
		addr_list = (struct in_addr **)mymachine->h_addr_list;
		for (i = 0; addr_list[i] != NULL; i++)
		{
			strcpy(symbolicip, inet_ntoa(* addr_list[i]));
			//printf("%s resolved to %s\n",mymachine->h_name ,symbolicip);
		}
	}


	struct sockaddr *serverptrCommand = (struct sockaddr *)&serverCommand;

	serverCommand.sin_family = AF_INET; 				/* Internet domain */
	serverCommand.sin_addr.s_addr = htonl(INADDR_ANY);
	serverCommand.sin_port = htons(command_port);		/* The given port */

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

	


	//printf("befoooooooooooooooooooooooooooooooooooore\n");
	//printAll(&urlQueue);
	printf("#Starting Crawling ...\n\n");

	/* threads */
	pthread_t HTMLdownloader[num_of_threads];
	pthread_mutex_init(&mtx, 0);
	pthread_mutex_init(&count_mtx, 0);
	pthread_cond_init(&all_serving, 0);
	for (i = 0; i < num_of_threads; i++){
		if ((err = pthread_create(&HTMLdownloader[i], 0, pthread_HTMLdownloader, 0))){
			perror2("pthread_create", err);
			exit(EXIT_FAILURE);
		}
	}
	
	
	struct sockaddr_in  serverSearch;
	struct sockaddr *serverSearchptr;
								

	serverSearchptr = (struct sockaddr *)&serverSearch;

	serverSearch.sin_family = AF_INET; 	
	memcpy(&serverSearch.sin_addr, mymachine->h_addr, mymachine->h_length);
	serverSearch.sin_port = htons(SEARCHPORT);	



	while(1){


		clientlen = 0;
		clientptr = (struct sockaddr *)&client;
		

		if ((newsockCommand = accept(sockCommand, clientptr, &clientlen)) < 0){
			perror_exit("accept");
		}

		if ((nrd = read(newsockCommand, bufCommand, sizeof(bufCommand))) > 0){

			//printf("bufCommand == '%s'\n", bufCommand);

			token = strtok_r(bufCommand, " \t\n", &argument);

			if ((strcmp(token, "GET") == 0) || (strcmp(token, "get") == 0)){
				token = strtok_r(argument, " /\t\n", &argument2);
				//printf("token --> '%s'\n", token);
				if (strcmp(token, "STATS") == 0){
					current_time = time(NULL);
					difference = current_time - start_time;
					pthread_mutex_lock(&count_mtx);
					printf("Crawler up for %ld:%ld.%ld, served %d pages, %ld bytes\n", (difference/3600), (difference/60), (difference%60), downloadedPages, downloadedBytes);
					pthread_mutex_unlock(&count_mtx);
					//close(newsockCommand);
				}
				else if (strcmp(token, "SHUTDOWN") == 0){

					if (first_time_search == 0){
						pthread_cond_broadcast(&all_serving);
						pthread_mutex_lock(&count_mtx);
						END_ALL = 1;
						pthread_mutex_unlock(&count_mtx);
						break;
					}
					else{
						
						if ((sockSearch = socket(AF_INET, SOCK_STREAM, 0)) < 0){
							perror_exit("socket");
						}

						int enableSocketADDR = 1;
						if (setsockopt(sockSearch, SOL_SOCKET, SO_REUSEADDR, &enableSocketADDR, sizeof(int)) < 0){
							perror_exit("setsockopt(SO_REUSEADDR)");
						}


						if (connect(sockSearch, serverSearchptr, sizeof(serverSearch)) < 0){
							printf("errno == %d\n", errno);
							perror_exit("connect");
						}

						char *sendMSG = (char *)malloc((strlen("/exit") + 1)*sizeof(char));
						if (sendMSG == NULL){
							perror_exit("malloc");
						}
						memset(sendMSG, '\0', ((strlen("/exit") + 1)));
						strcat(sendMSG, "/exit");
						printf("----->>>>>send buffer == '%s'\n", sendMSG);

						char test[20];
						memset(test, '\0', sizeof(test));
						sprintf(test, "%ld", strlen(sendMSG));
						for (int i = strlen(sendMSG); i < 19; i++){
							strcat(test, "!");
						}

						if ((nrd = write(sockSearch, test, sizeof(test))) < 0){
							perror_exit("write");
						}

						printf("send== '%s'\n", sendMSG);	
						printf("sizeof(sendMSG) == %ld\n", sizeof(sendMSG));
						printf("(strlen(\"/exit\") + 1) == %ld\n", strlen("/exit") + 1);
						if ((nrd = write(sockSearch, sendMSG, strlen("/exit"))) < 0){
							perror_exit("write");
						}

						if (sendMSG != NULL){
							free(sendMSG);
							sendMSG = NULL;
						}
						END_ALL = 1;
						break;
					}
				}
				else{ 
					//token = strtok(NULL, " /\t\n");
					//printf("token == '%s'\n", token);
					if (strcmp(token, "SEARCH") == 0){
						//not finished downloading
						i = 0;
						//printf("argument2 == '%s'\n", argument2);
						//token = strtok_r(argument2, " ", &argument3);
						token = strtok(argument2, " ");
						//printf("argument2 == '%s'\n", argument2);
						token = strtok(argument2, "/");
						//printf("%s\n", );
						while (token != NULL){
							//printf("token == '%s'\n", token);
							searchWords = (char **)realloc(searchWords, (i + 1)*sizeof(*searchWords));
							if (searchWords == NULL){
								perror_exit("realloc");
							}

							searchWords[i] = (char *)malloc((strlen(token) + 1)*sizeof(char));
							if (searchWords[i] == NULL){
								perror_exit("malloc");	
							}

							strcpy(searchWords[i], token);

							i++;
							//printf("i == %d\n", i);
							token = strtok(NULL, "/");
						}

						int j = i;

						if (i == 0){
							printf("Need arguments to run a SEARCH query...\n");
							continue;
						}

						i = 0;
						while (i < j){
							//printf("searchWords[%d] == '%s'\n", i, searchWords[i]);
							i++;
						}
						

						int lengthOfArguments = 0;
						int k = 0;
						while (k < i){
							lengthOfArguments += strlen(searchWords[k]);
							k++;
						}
																				/*1 == '\0' and k are spaces*/
						char *sendMSG = (char *)malloc((strlen("/search") + lengthOfArguments + 1 + k)*sizeof(char));
						if (sendMSG == NULL){
							perror_exit("malloc");
						}
						memset(sendMSG, '\0', (strlen("/search") + lengthOfArguments + 1 + k));
						k = 0;
						strcat(sendMSG, "/search");
						while(k < i){
							strcat(sendMSG, " ");
							strcat(sendMSG, searchWords[k]);
							k++;
						}
						printf("----->>>>>send buffer == '%s'\n", sendMSG);

						if (globalCounter == num_of_threads){

							/*create socket*/
							if ((sockSearch = socket(AF_INET, SOCK_STREAM, 0)) < 0){
								perror_exit("socket");
							}

							int enableSocketADDR = 1;
							if (setsockopt(sockSearch, SOL_SOCKET, SO_REUSEADDR, &enableSocketADDR, sizeof(int)) < 0){
								perror_exit("setsockopt(SO_REUSEADDR)");
							}


							if (connect(sockSearch, serverSearchptr, sizeof(serverSearch)) < 0){
								printf("errno == %d\n", errno);
								perror_exit("connect");
							}

							if (first_time_search == 0){	

								//waiting for threads
								for (i = 0; i < num_of_threads; i++){
									//printf("Thread will die...\n");
									if ((err = pthread_join(HTMLdownloader[i], NULL))){
										perror2("pthread_join", err);
										exit(EXIT_FAILURE);
									}
								}

								//printf("All threads terminated\n");

								if ((err = pthread_mutex_destroy(&count_mtx))){
									perror2("pthread_mutex_destroy (count_mtx)", err);
									exit(EXIT_FAILURE);
								}

								if ((err = pthread_mutex_destroy(&mtx))){
									perror2("pthread_mutex_destroy (mtx)", err);
									exit(EXIT_FAILURE);
								}

								if ((err = pthread_cond_destroy(&all_serving))){
									perror2("pthread_cond_destroy (all_serving)", err);
									exit(EXIT_FAILURE);
								}

								//creating log file
								getListOfLinks(&existHead2, &existHead, save_dir);
								deleteExistList(&existHead);
								//printf("booyah...\n");
								first_time_search = 1;
								fileName = makeFileName(&existHead2);
								deleteExistList(&existHead2);

							}


							char test[20];
							memset(test, '\0', sizeof(test));
							sprintf(test, "%ld", strlen(sendMSG));
							for (int i = strlen(sendMSG); i < 19; i++){
								strcat(test, "!");
							}

							if ((nrd = write(sockSearch, test, sizeof(test))) < 0){
								perror_exit("write");
							}

							printf("send== '%s'\n", sendMSG);	
							printf("sizeof(sendMSG) == %ld\n", sizeof(sendMSG));
							printf("(strlen(\"/search\") + lengthOfArguments + 1 + k) == %ld\n", (strlen("/search") + lengthOfArguments + 1 + k));
							if ((nrd = write(sockSearch, sendMSG, (strlen("/search") + lengthOfArguments + 1 + k) - 1)) < 0){
								perror_exit("write");
							}
							printf("send== '%s'\n", sendMSG);
							printf("strlen(sendMSG) == %ld\n", strlen(sendMSG));
							printf("nrd == %d\n", nrd);
							printf("after first write in Crawler\n");

							char read_byte[1000];
							//memset(read_byte, '\0', sizeof(read_byte));

							//memset(msgbuf, '\0', sizeof(msgbuf));
							/*
							char num_chars_read[10];	//size of 2d array
							int integer_num_chars_read;
							printf("sizeof(num_chars_read) == %lu\n", sizeof(num_chars_read));
							if ((nrd = read(sockSearch, num_chars_read, sizeof(num_chars_read))) < 0){
								perror_exit("read");
							}
							integer_num_chars_read = atoi(num_chars_read);
							printf("%s\n", num_chars_read);
							printf("into Crawler... integer_num_chars_read == %d\n", integer_num_chars_read);

							char **searchBuffer;
							char *templinesize;
							int integerTempLineSize = 0;
							char readCharOfSearch[2];
							//memset(searchBuffer, '\0', sizeof(searchBuffer));
							for (int i = 0; i < integer_num_chars_read; i++){
								if ((nrd = read(sockSearch, templinesize, sizeof(int))) < 0){
									perror_exit("read");
								}
								integerTempLineSize = atoi(templinesize);
								printf("integerTempLineSize == %d\n", integerTempLineSize);

								searchBuffer[i] = (char *)malloc((integerTempLineSize + 1)*sizeof(char));
								if (searchBuffer[i] == NULL){
									perror_exit("malloc");
								}

								memset(searchBuffer[i], '\0', sizeof(searchBuffer[i]));
								if ((nrd = read(sockSearch, searchBuffer[i], integerTempLineSize)) < 0){
									perror_exit("read");
								}
								//strcat(searchBuffer[i], readCharOfSearch);
							}

							printf("searchBuffer[0] == '%s'\n", searchBuffer[0]);
							*/
							printf("Search results are:\n");
							while (1){
								if ((nrd = read(sockSearch, read_byte, sizeof(read_byte))) < 0){
									perror_exit("read");
								}
								if (read_byte[0] == '#'){
									printf("\n");
									break;
								}
								printf("%s", read_byte);
								//memset(read_byte, '\0', sizeof(read_byte));
							}

							//memset(read_byte, '\0', sizeof(read_byte));




							
							/*
							printf("Search results are:\n");
							for (int i = 0; i < integer_num_chars_read; i++){
								printf("%s", searchBuffer[i]);
							}
							

							for (int i = 0; i < integer_num_chars_read; i++){
								if (searchBuffer[i] != NULL){
									free(searchBuffer[i]);
									searchBuffer[i] = NULL;
								}
							}
							if (searchBuffer != NULL){
								free(searchBuffer);
								searchBuffer = NULL;
							}*/


							close(sockSearch);

						}
						else{
							printf("Crawling is in progress. Please wait ...\n");
						}
						if (sendMSG != NULL){
							free(sendMSG);
							sendMSG = NULL;
						}

						for (int i = 0; i < strlen((char *)searchWords); i++){
							if (searchWords[i] != NULL){
								free(searchWords[i]);
								searchWords[i] = NULL;
							}
						}
						if (searchWords != NULL){
							free(searchWords);
							searchWords = NULL;
						}


					}
					else {
						printf("Unknown command \"%s\"of http protocol!\n", token);
					}
				}
			}
			
		}
		//close(newsockCommand);
	}
	
	close(newsockCommand);
	
	//waiting for threads
	if (first_time_search == 0){
		for (i = 0; i < num_of_threads; i++){
			//printf("Thread will die...\n");
			if ((err = pthread_join(HTMLdownloader[i], NULL))){
				perror2("pthread_join", err);
				exit(EXIT_FAILURE);
			}
		}

		printf("All threads terminated\n");

		if ((err = pthread_mutex_destroy(&count_mtx))){
			perror2("pthread_mutex_destroy (count_mtx)", err);
			exit(EXIT_FAILURE);
		}

		if ((err = pthread_mutex_destroy(&mtx))){
			perror2("pthread_mutex_destroy (mtx)", err);
			exit(EXIT_FAILURE);
		}

		if ((err = pthread_cond_destroy(&all_serving))){
			perror2("pthread_cond_destroy (all_serving)", err);
			exit(EXIT_FAILURE);
		}
	}



	

	if (save_dir != NULL){
		free(save_dir);
		save_dir = NULL;
	}

	printf(">>> Exiting mycrawler.c ...\n");
	pthread_exit(NULL);
}