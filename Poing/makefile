include $(HOME)/Makefiles/common.mk

MUTEX := ../mutex
SOCKET=../socket

EXES := Ping Pong
CARGS_Ping := -I$(SOCKET)
LARGS_Ping := $(SOCKET)/$(LIB)/libsocket.a $(MUTEX)/$(LIB)/libmutex.a -lpthread
CARGS_Pong := -I$(SOCKET)
LARGS_Pong := $(SOCKET)/$(LIB)/libsocket.a $(MUTEX)/$(LIB)/libmutex.a -lpthread

include $(TEMPLATES)/c.mk

