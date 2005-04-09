include $(HOME)/Makefiles/common.mk

LIBS := libsocket

OBJS_libsocket := socket.o
CARGS_socket := -DSOCKET_NO_EVT

include $(TEMPLATES)/c.mk

$(LIB)/libsocket.a $(LIB)/libsocket.so : $(OBJS_libsocket:%=$(LIB)/%)

socket.c : socket_prv.h
socket_prv.h : socket.h socket_net.h

