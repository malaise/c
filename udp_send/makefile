include $(HOME)/Makefiles/common.mk

MUTEX := ../mutex
SOCKET=../socket

EXES := udp_send

CARGS_udp_send := -I$(SOCKET)
LARGS_udp_send := $(SOCKET)/$(LIB)/libsocket.a $(MUTEX)/$(LIB)/libmutex.a -lpthread

include $(TEMPLATES)/c.mk

