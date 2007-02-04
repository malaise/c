include $(HOME)/Makefiles/common.mk

BOOLEAN=../boolean
TIMEVAL=../timeval
MUTEX=../mutex
SOCKET=../socket
SIG_UTIL=../sig_util

LIBS := libtty libudp
OBJS_libtty := tty.o
OBJS_libudp := udp.o
CARGS_udp := -I$(SOCKET)

EXES := serial_spy gorgy
CARGS_udp := -I$(BOOLEAN) -I$(SOCKET)
CARGS_serial_spy := -DSTANDARD
LIBS_serial_spy := libtty.a
CARGS_gorgy := -DSTANDARD -I$(BOOLEAN) -I$(TIMEVAL) -I$(SIG_UTIL)
LIBS_gorgy := libtty.a libudp.a
LARGS_gorgy := $(SOCKET)/$(LIB)/libsocket.a $(MUTEX)/$(LIB)/libmutex.a \
               $(TIMEVAL)/$(LIB)/libtimeval.a -lpthread -lm

include $(TEMPLATES)/c.mk

