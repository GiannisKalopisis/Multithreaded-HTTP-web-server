
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
#include <libgen.h>

int main(int argc, char const *argv[])
{
	/* code */
	/*
	char path[30] = "./site1/page1_100.html";
	char *dname = dirname(path);
	char *bname = basename(dname);

	printf("dname == '%s'\n", bname);*/
	char searchBuffer[20];
	printf("sizeof of searchBuffer before memset == %ld\n", sizeof(searchBuffer));

	memset(searchBuffer, '\0', sizeof(searchBuffer));
	printf("sizeof of searchBuffer after memset == %ld\n", sizeof(searchBuffer));

	return 0;
}