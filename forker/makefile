include $(HOME)/Makefiles/common.mk

BOOLEAN=../boolean
SOCKET=../socket

EXES := forker client
CARGS_forker := -I$(BOOLEAN) -I$(SOCKET)
LARGS_forker := $(SOCKET)/$(LIB)/libsocket.a
CARGS_client := -I$(BOOLEAN) -I$(SOCKET)
LARGS_client := $(SOCKET)/$(LIB)/libsocket.a -lget_line

include $(TEMPLATES)/c.mk

forker.c : forker_messages.h

