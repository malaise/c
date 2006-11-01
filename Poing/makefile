include $(HOME)/Makefiles/common.mk

BOOLEAN := ../boolean
MUTEX := ../mutex
SOCKET=../socket

EXES := Ping Pong
CARGS_Ping := -I$(BOOLEAN) -I$(SOCKET)
LARGS_Ping := $(SOCKET)/$(LIB)/libsocket.a $(MUTEX)/$(LIB)/libmutex.a -lpthread
CARGS_Pong := -I$(BOOLEAN) -I$(SOCKET)
LARGS_Pong := $(SOCKET)/$(LIB)/libsocket.a $(MUTEX)/$(LIB)/libmutex.a -lpthread

include $(TEMPLATES)/c.mk

