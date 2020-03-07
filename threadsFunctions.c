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
#include "structCrawler.h"
#include "threadsFunctions.h"
#include "generalFunctions.h"
#include "generalFunctionsCrawler.h"


void *pthread_HTMLdownloader(void *ptr){

	char *url, *finalurl, buf[2000], *msgbuf = NULL, **paths;//, buf2[300000];
	char *myURL = NULL, char_read[2], *ret, *argument, *token, *bytes1;
	int nrd, connect_fd, bytes, sock, i = 0, strlen_of_paths;

	struct sockaddr_in  server;
	struct sockaddr *serverptr;
	

	serverptr = (struct sockaddr *)&server;

	server.sin_family = AF_INET; 	/* Internet domain */
	memcpy(&server.sin_addr, mymachine->h_addr, mymachine->h_length);
	server.sin_port = htons(port);	/* Server port */


	while (1){
		

		//printAll(&urlQueue);

		/*create socket*/
		if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){
			perror_exit("socket");
		}
		int enableSocketADDR = 1;
		if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enableSocketADDR, sizeof(int)) < 0){
			perror_exit("setsockopt(SO_REUSEADDR)");
		}


		if (connect(sock, serverptr, sizeof(server)) < 0){
			printf("errno == %d\n", errno);
			perror_exit("connect");
		}


		if (END_ALL == 1){
			close(sock);
			pthread_cond_broadcast(&all_serving);
			pthread_mutex_lock(&count_mtx);
			globalCounter++;
			pthread_mutex_unlock(&count_mtx);
			pthread_exit(0);
		}


		pthread_mutex_lock(&mtx);
		if (END_ALL == 1) {
			//printf("yes\n");
			close(sock);
			pthread_cond_broadcast(&all_serving);
			pthread_mutex_unlock(&mtx);
			printf("Thread %ld will terminate...\n", pthread_self());
			pthread_mutex_lock(&count_mtx);
			globalCounter++;
			pthread_mutex_unlock(&count_mtx);
			pthread_exit(0);
		}
		//printf("yeah\n");
		while ((url = getFrontURL(&urlQueue)) == NULL){
			//printf("++++++++++++++++++++++++++++\n");
			serving_threads++;
			//printf("serving_threads == %d\n", serving_threads);
			if ((serving_threads == num_of_threads) || (END_ALL == 1)) {
				//printf("yes\n");
				close(sock);
				pthread_cond_broadcast(&all_serving);
				pthread_mutex_unlock(&mtx);
				printf("Thread %ld will terminate...\n", pthread_self());
				pthread_mutex_lock(&count_mtx);
				globalCounter++;
				pthread_mutex_unlock(&count_mtx);
				pthread_exit(0);
			}
			pthread_cond_wait(&all_serving, &mtx);
			if ((serving_threads == num_of_threads) || (END_ALL == 1)) {
				close(sock);
				pthread_cond_broadcast(&all_serving);
				pthread_mutex_unlock(&mtx);
				printf("Thread %ld will terminate...\n", pthread_self());
				pthread_mutex_lock(&count_mtx);
				globalCounter++;
				pthread_mutex_unlock(&count_mtx);
				pthread_exit(0);
			}
			serving_threads--;
		}
		myURL = (char *)malloc((strlen(url) + 1)*sizeof(char));
		if (myURL == NULL){
			perror_exit("malloc myURL");
		}
		//printf("url == '%s'\n", url);
		strcpy(myURL, url);
		//printAll(&urlQueue);
		pop(&urlQueue);
		pthread_mutex_unlock(&mtx);
		//printf("ok\n");



		//printf("url ----->'%s'\n", myURL);
		//finalurl = getURL(url);
		//printf("finalurl == '%s'\n", finalurl);

		snprintf(buf, sizeof(buf), GET_REQUEST, myURL, symbolicip);
		//printf("buf == '%s'\n", buf);
		
		if (write(sock, buf, strlen(buf)) < 0){
			perror_exit("write");
		}
		int counter = 0;
		buf[0] = '\0';

		while ((nrd = read(sock, char_read, sizeof(char))) > 0){

			strcat(buf, char_read); 
			counter++;
			if (counter >= 4){
				//printf("counter == %d\n", counter);
				if (buf[counter-3] == '\n' && buf[counter-1] == '\n'){
					//printf("buf == '%s'\n", buf);
					ret = strstr(buf, "Content-Length:");
					if (ret == NULL){
						printf("Somethinq went wrong with HTTP reply protocol!\n");
						exit(EXIT_FAILURE);
					}
					else{
						token = strtok_r(ret, " \r\t\n", &argument);
						bytes1 = strtok(argument, " \r\t\n");
						//printf("bytes1 == '%s'\n", bytes1);
						bytes = atoi(bytes1);
						//printf("bytes == %d\n", bytes);
						msgbuf = (char *)malloc((bytes + 1)*sizeof(char));
						if (msgbuf == NULL){
							perror_exit("malloc");
						}
					}
					break;
				}
			}
		}

		pthread_mutex_lock(&count_mtx);
		downloadedBytes += bytes;
		downloadedPages++;
		if (myURL[0] == '.'){
			printf("#Downloading page %s...\n", myURL);
		}
		else printf("#Downloading page .%s...\n", myURL);
		pthread_mutex_unlock(&count_mtx);


		counter = 0;
		while ((nrd = read(sock, char_read, sizeof(char))) > 0){
			counter++;
			if (counter == 8){
				break;
			}
		}

		counter = 0;
		memset(msgbuf, '\0', sizeof(msgbuf));
		while ((nrd = read(sock, char_read, sizeof(char))) > 0){
			strcat(msgbuf, char_read);
			counter++;
			if (counter >= bytes){
				break;
			}
		}

		//printf("msgbuf == \n'%s'\n", msgbuf);
		finalurl = createFilePath(myURL, save_dir);
		//printf("finalurl == '%s'\n", finalurl);

		FILE *fp = fopen(finalurl, "w+");
		if (fp == NULL){
			perror_exit("fopen");
		}
		fprintf(fp, "%s", msgbuf);
		fclose(fp);
		if (finalurl != NULL){
			free(finalurl);
			finalurl = NULL;
		}

		paths = getLinksFromFile(msgbuf);
		//printf("sizeof(paths)== %ld\n", strlen(paths));

		pthread_mutex_lock(&mtx);
		i = 0;
		while (i < strlen((char *)paths)){
			push(&urlQueue, paths[i], &existHead);
			pthread_cond_broadcast(&all_serving);
			i++;
		}

		i = 0;
		strlen_of_paths = strlen((char *)paths);
		while (i < strlen_of_paths){
			if (paths[i] != NULL){
				free(paths[i]);
				paths[i] = NULL;
			}
			i++;
		}
		if (paths != NULL){
			free(paths);
			paths = NULL;
		}


		pthread_mutex_unlock(&mtx);

		//printAll(&urlQueue);
		//printf("======================================\n");
		if (myURL != NULL){
			free(myURL);
			myURL = NULL;
			//printf("ook\n");
		}

		if (msgbuf != NULL){
			free(msgbuf);
			msgbuf = NULL; 
		}
		//break;
		close(sock);

		//printf("new loop...\n");
	}

	pthread_mutex_lock(&count_mtx);
	globalCounter++;
	pthread_mutex_unlock(&count_mtx);
	pthread_exit(0);	
}


void initialize(pool_t *pool){
	pool->start = 0;
	pool->end = -1;
	pool->count = 0;
	//pool->exit_var = 0;
	return;
}


void place(pool_t *pool, int fileDesc){
	//printf("I am thread %ld\n", );
	pthread_mutex_lock(&mtx);
	while (pool->count >= POOL_SIZE){
		printf(">>> Found Buffer Full\n");
		pthread_cond_wait(&cond_nonfull, &mtx);
	}
	pool->end = (pool->end + 1) % POOL_SIZE;
	pool->data[pool->end] = fileDesc;
	pool->count++;
	pthread_cond_signal(&cond_nonempty);
	pthread_mutex_unlock(&mtx);
	return;
}


int obtain(pool_t *pool){
	printf("I am new thread with id: %ld\n", pthread_self());
	int fileDesc = -1;
	pthread_mutex_lock(&mtx);
	while (pool->count <= 0){
		printf(">>> Found Buffer Empty\n");
		pthread_cond_wait(&cond_nonempty, &mtx);
	}
	fileDesc = pool->data[pool->start];
	printf("I am thread %ld and i just took fd=%d from place %d\n", pthread_self(), fileDesc, pool->start);
	pool->start = (pool->start + 1) % POOL_SIZE;
	pool->count--;
	pthread_cond_signal(&cond_nonfull);
	pthread_mutex_unlock(&mtx);
	return fileDesc;
}


void *pthread_consumer(void *ptr){
	int newsock, lengthOfFile, headerLength, counter, nrd, count = 0;
	char buf[1600], *token, *argument, headerReturn[1000], date[35], *msgSend, symbol, finalPath[300];
	FILE *fp;
	struct tm *sTm;
	time_t now;

	char *dname, *bname, *bnameOfDir, *path, *dirPath;

	


	while(1){
		//if (pool.count > 0){
			newsock = obtain(&pool);
			if (newsock == -1){
				printf("newsock == %d\n", newsock);
				printf("I am thread %ld and I will put -1 at pool.\n", pthread_self());
				//place(&pool, -1);
				place(&pool, -1);

				pthread_exit(0);
				//count++;
				//if (count == 2){
				//	pthread_exit(0);
				//}
			}
			else if ((nrd = read(newsock, buf, sizeof(buf))) > 0){ 
				//printf("--------------------------\n");
				//printf("\n%s\n", buf);
				token = strtok_r(buf, " \t\n", &argument);
				if ((strcmp(token, "GET") == 0) || (strcmp(token, "get") == 0)){
					token = strtok(argument, " \t\n");
					//close(sock);
					printf("token == '%s'\n", token);
					printf("path == '%s'\n", token);

					bname = basename(token);
					//printf("basename == '%s'\n", bname);
					dname = dirname(token);
					//printf("dirname == '%s'\n", dname);
					bnameOfDir = basename(dname);

					for (int i = 0; i < 300; ++i){
						finalPath[i] = '\0';
					}
					strcat(finalPath, "./");
					strcat(finalPath, root_dir);
					strcat(finalPath, "/");
					strcat(finalPath, bnameOfDir);
					strcat(finalPath, "/");
					strcat(finalPath, bname);


					printf("finalPath == '%s'\n", finalPath);

					if (access(finalPath, F_OK) != -1){
						if (access(finalPath, R_OK) != -1){
							fp = fopen(finalPath, "r");
							if (fp != NULL){
								fseek(fp, 0, SEEK_END);
								lengthOfFile = ftell(fp);
								//printf("lengthOfFile == %d\n", lengthOfFile);
								fseek(fp, 0, SEEK_SET);
								fclose(fp);
							}
							else{
								perror_exit("fopen");
							}
							now = time(0);
							sTm = gmtime(&now);
							strftime(date, sizeof(date), "%a, %d %b %Y %X %Z", sTm);
							snprintf(headerReturn, sizeof(headerReturn), HEADER, "200 OK", date, lengthOfFile);
							headerLength = strlen(headerReturn);

							msgSend = (char *)malloc((lengthOfFile + headerLength + strlen(HEAD_START) + strlen(HEAD_END) + 1)*sizeof(char));
							if (msgSend == NULL){
								perror_exit("malloc");
							}

							msgSend[0] = '\0';
							strcat(msgSend, headerReturn);
							strcat(msgSend, HEAD_START);
							//printf("'''\n%s\n'''\n", msgSend);
							fp = fopen(finalPath, "r");
							if(fp != NULL){
								//printf("skata\n");
								counter = 0;
								//printf("strlen(msgSend) == %ld\n", strlen(msgSend));
								counter = strlen(msgSend);
								while((symbol = getc(fp)) != EOF){
									msgSend[counter + 1] = '\0';
									msgSend[counter] = symbol;
									counter++;
									strcat(msgSend, &symbol);
								}
								//printf("path == '%s'\n", finalPath);
								msgSend[counter] = '\0';
							}
							else {
								perror_exit("fopen");
							}

							strcat(msgSend, HEAD_END);
							fclose(fp);

							if (write(newsock, msgSend, strlen(msgSend)) < 0){
								perror_exit("write");
							}

							close(newsock);

							pthread_mutex_lock(&count_mtx);
							servedPages++;
							servedBytes+=lengthOfFile;
							pthread_mutex_unlock(&count_mtx);


							if (msgSend != NULL){
								free(msgSend);
								msgSend = NULL;
							}
						}
						else{

							//no read permissions
							now = time(0);
							sTm = gmtime(&now);
							strftime(date, sizeof(date), "%a, %d %b %Y %X %Z", sTm);
							lengthOfFile = strlen(FORBIDDEN);
							snprintf(headerReturn, sizeof(headerReturn), HEADER, "403 Forbidden", date, lengthOfFile);
							headerLength = strlen(headerReturn);

							msgSend = (char *)malloc((lengthOfFile + headerLength)*sizeof(char));
							if (msgSend == NULL){
								perror_exit("malloc");
							}

							msgSend[0] = '\0';
							strcat(msgSend, headerReturn);
							strcat(msgSend, FORBIDDEN);

							if (write(newsock, msgSend, strlen(msgSend)) < 0){
								perror_exit("write");
							}

							close(newsock);

							if (msgSend != NULL){
								free(msgSend);
								msgSend = NULL;
							}
						}
					}
					else{
						//dont exists
						now = time(0);
						sTm = gmtime(&now);
						strftime(date, sizeof(date), "%a, %d %b %Y %X %Z", sTm);
						lengthOfFile = strlen(NOT_EXISTS);
						//printf("HEADER == %s\n", HEADER);
						snprintf(headerReturn, sizeof(headerReturn), HEADER, "404 Not Found", date, lengthOfFile);
						headerLength = strlen(headerReturn);
						msgSend = (char *)malloc((lengthOfFile + headerLength)*sizeof(char));
						if (msgSend == NULL){
							perror_exit("malloc");
						}
						msgSend[0] = '\0';
						strcat(msgSend, headerReturn);
						strcat(msgSend, NOT_EXISTS);
						//printf("%s\n", msgSend);

						if (write(newsock, msgSend, strlen(msgSend)) < 0){
							perror_exit("write");
						}

						close(newsock);

						if (msgSend != NULL){
							free(msgSend);
							msgSend = NULL;
						}	
					}
				}
				else {
					fprintf(stderr, "Unknown request for http protocol!\n");
				}
			}
			//close(newsock);
		//}
	}
	pthread_exit(0);
}

