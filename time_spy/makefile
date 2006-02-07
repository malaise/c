include $(HOME)/Makefiles/common.mk
BOOLEAN=../boolean
TIMEVAL=../timeval
MUTEX := ../mutex
SOCKET=../socket

EXES := time_spy

CARGS_time_spy := -I$(BOOLEAN) -I$(TIMEVAL) -I$(SOCKET)
LARGS_time_spy := $(SOCKET)/$(LIB)/libsocket.a $(MUTEX)/$(LIB)/libmutex.a $(TIMEVAL)/$(LIB)/libtimeval.a -lpthread -lm

include $(TEMPLATES)/c.mk

