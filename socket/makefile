include $(HOME)/Makefiles/common.mk

LIBS := libsocket

OBJS_libsocket := socket.o
CARGS_socket := -DSOCKET_NO_EVT

include $(TEMPLATES)/c.mk

socket.c : socket_prv.h
socket_prv.h : socket.h socket_net.h

