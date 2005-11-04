include $(HOME)/Makefiles/common.mk

BOOLEAN=../boolean
MUTEX=../mutex

LIBS := libsocket

OBJS_libsocket := socket.o socket_afux.o
CARGS_socket := -DSOCKET_NO_EVT -DSOCKET_MUTEX -I$(BOOLEAN) -I$(MUTEX)

include $(TEMPLATES)/c.mk

