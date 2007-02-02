include $(HOME)/Makefiles/common.mk

BOOLEAN=../boolean
SOCKET=../socket

EXES := udpspy
LIBS := libutil

OBJS_libutil := util.o

LARGS_udpspy := $(LIB)/libutil.a $(SOCKET)/$(LIB)/libsocket.a -lpthread
CARGS_util := -I$(BOOLEAN) -I$(SOCKET)

include $(TEMPLATES)/c.mk

