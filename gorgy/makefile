include $(HOME)/Makefiles/common.mk

SOCKET=../socket
SIG_UTIL=../sig_util

LIBS := libtty libudp
OBJS_libtty := tty.o 
OBJS_libudp := udp.o 
CARGS_udp := -I$(SOCKET)

EXES := serial_spy gorgy
CARGS_serial_spy := -DSTANDARD
LIBS_serial_spy := libtty.a
CARGS_gorgy := -DSTANDARD -I$(SIG_UTIL)
LIBS_gorgy := libtty.a libudp.a
LARGS_gorgy := $(SOCKET)/$(LIB)/libsocket.a -ltimeval

include $(TEMPLATES)/c.mk

udp.c : udp.h
tty.c : tty.h
serial_spy.c : tty.h
gorgy.c : tty.h udp.h

