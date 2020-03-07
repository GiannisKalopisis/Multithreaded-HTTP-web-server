#ifndef STRUCTS_H
#define STRUCTS_H


typedef struct fifo_Names FifoNames;
typedef struct listNode ListNode;
typedef struct node Node;
typedef struct postingListWordInfo PostingListWordInfo;
typedef struct map Map;
typedef struct passing_arguments passingArgumentsStruct;
typedef struct send_struct sendStruct;
typedef struct passing_query_length passingQueryLength;
//typedef struct dir_send dir_Send;
//typedef struct search_struct search_Struct;
typedef struct max_min_send_struct MaxMinSendStruct;
//typedef struct min_struct min_Struct;
typedef struct wc_struct wc_Struct;
typedef struct max_min_count_struct MaxMinCountStruct;
typedef struct search_Info_Struct searchInfoStruct;
typedef struct starting_search_struct startingSearchStruct;


struct fifo_Names{
	int fd_server_client;
	int fd_client_server;
	char *serverName;	//server writes here and client reads here
	char *clientName;	//server reads here and client writes here
};


struct listNode{
	char *nameOfFile;
	int wordCounterInFile;
	PostingListWordInfo *wordInfoStruct;
	ListNode *next;
};


struct node{
	char letter;
	ListNode *end;
	Node *child;
	Node *next;
};


struct postingListWordInfo{
	int numberOfLine;
//	int offsetOfLine;
	PostingListWordInfo *next;	
};


struct map{
	char *filePath;
	int *searchQuery;
	int numLines;
	char **text;
};


struct passing_arguments{
	int numOfArguments;
	char **Arguments;
};


struct send_struct{
	int numOfArguments;
	int maxDirSize;
};


struct passing_query_length{
	int lengthOfQuery;
};


struct max_min_count_struct{
	int timesWordExist;
	char *fullFilePath;
};

/*
struct dir_send{
	int sizeOfNextArgument;
	char *directory;
};
*/

/*
struct search_struct{
	int intoDeadline;
	char **answerArray;
	int numberOfLinesInAnswerArray;
};
*/

struct max_min_send_struct{
	int wordCount;
	int lengthOfFilePath;
};

/*
struct min_struct{
	int minExistingTimes;
	char *file;
};
*/


struct wc_struct{
	int bytesNum;
	int wordsNum;
	int linesNum;
};


struct search_Info_Struct{
	int sizeOfPath;
	int numberOfLine;
	int sizeOfLine;
};


struct starting_search_struct{
	int numberOfResults;
	int intoDeadline;
};


#endif
