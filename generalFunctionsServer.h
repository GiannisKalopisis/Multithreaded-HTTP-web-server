#ifndef GENERALFUNCTIONS_H
#define GENERALFUNCTIONS_H



int isNumber(char const *numWorkers);

void checkParameters(int argc, char const **argv);

void perror_exit(char *message);

char *getHeader(char *protocolReturn, int lengthOfContent);

#endif