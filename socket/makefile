CC=gcc
CCOPT=-g
BOOLEAN=../boolean

socket.o : socket.c socket.h
	$(CC) $(CCOPT) -DSOCKET_NO_EVT -I$(BOOLEAN) -c socket.c

