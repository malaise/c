include $(HOME)/Makefiles/common.mk

SOCKET=../socket

EXES := Ping Pong
CARGS_Ping := -I$(SOCKET)
LARGS_Ping := $(SOCKET)/$(LIB)/libsocket.a
CARGS_Pong := -I$(SOCKET)
LARGS_Pong := $(SOCKET)/$(LIB)/libsocket.a

include $(TEMPLATES)/c.mk

Ping.c : Poing.h
Pong.c : Poing.h

