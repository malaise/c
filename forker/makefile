CC=gcc
CCOPT=-g
BOOLEAN=../boolean
TIMER=../timer
UDP=../udp

all : forker client

forker : forker.o
	$(CC) $(CCOPT) $(TIMER)/timer.o $(UDP)/soc.o -o forker forker.o

forker.o : forker.c forker_messages.h makefile
	$(CC) $(CCOPT) -I$(BOOLEAN) -I$(TIMER) -I$(UDP) -c forker.c

client : client.o
	$(CC) $(CCOPT) $(TIMER)/timer.o $(UDP)/soc.o -lget_line -o client client.o

client.o : client.c forker_messages.h makefile
	$(CC) $(CCOPT) -I$(BOOLEAN) -I$(TIMER) -I$(UDP) -c client.c

clean :
	rm -f *.o

clean_exe :
	rm -f forker client

