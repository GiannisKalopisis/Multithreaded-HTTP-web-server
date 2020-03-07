#ifndef STRUCTS_H
#define STRUCTS_H


typedef struct head Queue_Head;
typedef struct url_queue URL_Queue;
typedef struct existhead Exist_Head;
typedef struct exist_queue exist_Queue;


struct head{
	int queue_size;
	URL_Queue *Front;
	URL_Queue *Last;
};


struct url_queue{
	int serving;
	char *url;
	URL_Queue *next;
};


struct existhead{
	int queue_size;
	exist_Queue *First;
};


struct exist_queue{
	char *myPath;
	exist_Queue *next;	
};


#endif
