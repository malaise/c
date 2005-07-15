include $(HOME)/Makefiles/common.mk

MUTEX := ../mutex
SOCKET=../socket

EXES := time_spy

CARGS_time_spy := -I$(SOCKET)
LARGS_time_spy := $(SOCKET)/$(LIB)/libsocket.a $(MUTEX)/$(LIB)/libmutex.a -ltimeval -lpthread

include $(TEMPLATES)/c.mk

