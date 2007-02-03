include $(HOME)/Makefiles/common.mk

BOOLEAN=../boolean
SOCKET=../socket
MUTEX=../mutex
TIMEVAL=../timeval
WAIT_EVT=../wait_evt

EXES := udpspy
LIBS := libutil

OBJS_libutil := util.o

CARGS_udpspy := -I$(BOOLEAN) -I$(SOCKET) -I$(TIMEVAL) -I$(WAIT_EVT)
LARGS_udpspy := $(LIB)/libutil.a \
                $(SOCKET)/$(LIB)/libsocket.a $(MUTEX)/$(LIB)/libmutex.a \
                $(WAIT_EVT)/$(LIB)/libwait_evt.a $(TIMEVAL)/$(LIB)/libtimeval.a \
                -lpthread -lm
CARGS_util := -I$(BOOLEAN) -I$(SOCKET) -I$(TIMEVAL)

include $(TEMPLATES)/c.mk

