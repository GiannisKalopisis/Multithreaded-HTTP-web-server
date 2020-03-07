LIBS	= -lm
FLAGS	= -c
THREADF = -lpthread
CC	= gcc
SOBJS	= myhttpd.o generalFunctions.o generalFunctionsCrawler.o threadsFunctions.o
COBJS	= mycrawler.o generalFunctions.o generalFunctionsCrawler.o threadsFunctions.o
WOBJS	= workerCode.o generalFunctionsJE.o TrieFunctions.o
JOBJS	= jobExecutor.o generalFunctionsJE.o



all: jobExecutor workerCode myhttpd mycrawler


mycrawler: $(COBJS)
	$(CC) -g $(COBJS) -o $@ $(THREADF)

myhttpd: $(SOBJS)
	$(CC) -g $(SOBJS) -o $@ $(THREADF)

myhttpd.o: myhttpd.c
	$(CC) $(FLAGS) myhttpd.c

mycrawler.o: mycrawler.c
	$(CC) $(FLAGS) mycrawler.c

generalFunctionsCrawler.o: generalFunctionsCrawler.c
	$(CC) $(FLAGS) generalFunctionsCrawler.c

generalFunctions.o: generalFunctions.c
	$(CC) $(FLAGS) generalFunctions.c

threadsFunctions.o: threadsFunctions.c
	$(CC) $(FLAGS) threadsFunctions.c

workerCode: $(WOBJS)
	$(CC) $(WOBJS) -o $@

workerCode.o: workerCode.c
	$(CC) $(FLAGS) workerCode.c

generalFunctionsJE.o: generalFunctionsJE.c
	$(CC) $(FLAGS) generalFunctionsJE.c

TrieFunctions.o: TrieFunctions.c
	$(CC) $(FLAGS) TrieFunctions.c

jobExecutor: $(JOBJS)
	$(CC) $(JOBJS) -o $@




cleanAll: cleanPipes cleanLogFile clean

clean:
	rm -f *.o

cleanLogFile:
	rm -f ./log/*

cleanPipes:
	rm -f fifo*









