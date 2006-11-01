include $(HOME)/Makefiles/common.mk

BOOLEAN=../boolean
MUTEX=../mutex
SOCKET=../socket

EXES := name_of

CARGS_name_of := -I$(BOOLEAN) -I$(SOCKET)
LARGS_name_of := $(SOCKET)/$(LIB)/libsocket.a $(MUTEX)/$(LIB)/libmutex.a -lpthread

include $(TEMPLATES)/c.mk

