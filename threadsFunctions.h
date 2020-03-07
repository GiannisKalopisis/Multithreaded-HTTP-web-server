#ifndef THEADSFUNCTIONS_H
#define THEADSFUNCTIONS_H


#include "threadsFunctions.h"
#include "structCrawler.h"
#include "variables.h"
#include "generalFunctions.h"


#define SEARCHPORT 8899

#define perror2(s, e) fprintf(stderr , "%s: %s\n", s, strerror(e))

#define HEADER\
	"HTTP/1.1 %s\r\n"\
	"Date: %s\r\n"\
	"Server: myhttp/1.0.0 (Ubuntu64)\r\n"\
	"Content-Length: %d\r\n"\
	"Content-Type: text/html\r\n"\
	"Connection: Closed\r\n\r\n"

#define HEAD_START "<html>\r\n"

#define HEAD_END "\r\n</html>"

#define	NOT_EXISTS "<html>Sorry dude, couldn't find this file.</html>"

#define FORBIDDEN "<html>Trying to access this file but don't think i can make it.</html>"

#define GET_REQUEST "GET %s HTTP/1.1\r\n"\
					"Host: %s\r\n"

#define perror2(s, e) fprintf(stderr , "%s: %s\n", s, strerror(e))

#define POOL_SIZE 50




void *pthread_HTMLdownloader(void *ptr);

void initialize(pool_t *pool);

void place(pool_t *pool, int fileDesc);

int obtain(pool_t *pool);

void *pthread_consumer(void *ptr);





#endif