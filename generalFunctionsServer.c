#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


#define HEADER\
	"HTTP/1.1 %s\r\n"\
	"Date: %s\r\n"\
	"Server: myhttp/1.0.0 (Ubuntu64)\r\n"\
	"Content-Length: %d\r\n"\
	"Content-Type: text/html\r\n"\
	"Connection: Closed\r\n\r\n"



int isNumber(char const *numWorkers){
	int length = strlen(numWorkers);
	for (int i = 0; i < length; i++){
		if ((numWorkers[i] < '0') || (numWorkers[i] > '9')){
			return 0;
		}
	}
	return 1;
}


void checkParameters(int argc, char const **argv){

	if (argc < 9){
		fprintf(stderr, "Not enough arguments!\n");
		exit(EXIT_FAILURE);
	}
	else{

		//checking for -p, -c, -t, -d parameters
		if ((strcmp(argv[1], "-p") != 0)){
			fprintf(stderr, "Wrong parameter for serving_port!\n");
			exit(EXIT_FAILURE);
		}
		if ((strcmp(argv[3], "-c") != 0)){
			fprintf(stderr, "Wrong parameter for command_port!\n");
			exit(EXIT_FAILURE);
		}
		if ((strcmp(argv[5], "-t") != 0)){
			fprintf(stderr, "Wrong parameter for num_of_threads!\n");
			exit(EXIT_FAILURE);
		}
		if ((strcmp(argv[7], "-d") != 0)){
			fprintf(stderr, "Wrong parameter for root_dir!\n");
			exit(EXIT_FAILURE);
		}

		//checking if parameters are numbers
		if (!isNumber(argv[2])){
			fprintf(stderr, "Serving_port must be a positive integer!\n");
			exit(EXIT_FAILURE);
		}

		if (!isNumber(argv[4])){
			fprintf(stderr, "Command_port must be a positive integer!\n");
			exit(EXIT_FAILURE);
		}

		if (!isNumber(argv[6])){
			fprintf(stderr, "Num_of_threads must be a positive integer!\n");
			exit(EXIT_FAILURE);
		}

		//checking if service and command ports are the same
		if (strcmp(argv[2], argv[4]) == 0){
			fprintf(stderr, "Serving_port and Command_port must be different positive integers\n");
			exit(EXIT_FAILURE);
		}

		//checking if root_dir is directory
		struct stat path_stat;
    	stat(argv[8], &path_stat);
    	if (!S_ISDIR(path_stat.st_mode)){
    		fprintf(stderr, "Root_dir must be a directory!\n");
    		exit(EXIT_FAILURE);
    	}
	}

	return;
}


void perror_exit(char *message){
	perror(message);
	exit(EXIT_FAILURE);
}


char *getHeader(char *protocolReturn, int lengthOfContent){

	char *protocolHeaderReturn = (char *)malloc(1000*(sizeof(char)));
	if (protocolHeaderReturn == NULL){
		perror_exit("malloc");
	}
	char protocolHeader[1000];

	char date[35];
	struct tm *sTm;
	time_t now = time(0);
	sTm = gmtime(&now);
	strftime (date, sizeof(date), "%a, %d %b %Y %X %Z", sTm);

	printf("HEADER == %s\n", HEADER);
	snprintf(protocolHeader, sizeof(protocolHeader), HEADER, protocolReturn, date, lengthOfContent);
	//protocolHeader[999] = '\0';
	printf("protocolHeader == \n%s\n%ld\n", protocolHeader, strlen(protocolHeader));
	protocolHeaderReturn = protocolHeader;
	return protocolHeaderReturn;
}


