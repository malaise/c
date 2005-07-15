include $(HOME)/Makefiles/common.mk

BOOLEAN=../boolean
MUTEX=../mutex

LIBS := libsocket

OBJS_libsocket := socket.o socket_afux.o
CARGS_socket := -DSOCKET_NO_EVT -DSOCKET_MUTEX -I$(BOOLEAN) -I$(MUTEX)

include $(TEMPLATES)/c.mk

socket.c : socket_prv.h socket_afux.h
socket_afux.c : socket_afux.h
socket_afux.h : socket_net.h
socket_prv.h : socket.h socket_net.h

