CC=gcc
CCOPT=-g
BOOLEAN=../boolean
TIMER=../timer
SOCKET=../socket

all : forker client

forker : forker.o
	$(CC) $(CCOPT) $(TIMER)/timer.o $(SOCKET)/socket.o -o forker forker.o

forker.o : forker.c forker_messages.h makefile
	$(CC) $(CCOPT) -I$(BOOLEAN) -I$(TIMER) -I$(SOCKET) -c forker.c

client : client.o
	$(CC) $(CCOPT) $(TIMER)/timer.o $(SOCKET)/socket.o -lget_line -o client client.o

client.o : client.c forker_messages.h makefile
	$(CC) $(CCOPT) -I$(BOOLEAN) -I$(TIMER) -I$(SOCKET) -c client.c

clean :
	rm -f *.o

clean_exe :
	rm -f forker client

