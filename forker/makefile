include $(HOME)/Makefiles/common.mk

BOOLEAN=../boolean
SOCKET=../socket
MUTEX=../mutex

EXES := forker client
CARGS_forker := -I$(BOOLEAN) -I$(SOCKET)
LARGS_forker := $(SOCKET)/$(LIB)/libsocket.a $(MUTEX)/$(LIB)/libmutex.a -lpthread
CARGS_client := -I$(BOOLEAN) -I$(SOCKET)
LARGS_client := $(SOCKET)/$(LIB)/libsocket.a $(MUTEX)/$(LIB)/libmutex.a -lget_line -lpthread

include $(TEMPLATES)/c.mk

