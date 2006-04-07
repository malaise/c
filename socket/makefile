include $(HOME)/Makefiles/common.mk

BOOLEAN=../boolean
MUTEX=../mutex

FILES2LINK := socket.h socket_afux.h socket_prv.h socket_afux.c socket_net.h \
              socket.c
LINKFROM := $(HOME)/ada/c

LIBS := libsocket

OBJS_libsocket := socket.o socket_afux.o
CARGS_socket := -DSOCKET_NO_EVT -DSOCKET_MUTEX -I$(BOOLEAN) -I$(MUTEX)

include $(TEMPLATES)/c.mk

