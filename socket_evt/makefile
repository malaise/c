include $(HOME)/Makefiles/common.mk

BOOLEAN := ../boolean
TIMEVAL := ../timeval
WAIT_EVT := ../wait_evt

FILES2LINK := socket.h socket_afux.h socket_prv.h socket_afux.c socket_net.h \
              socket.c
LINKFROM := $(HOME)/ada/c

LIBS := libsocket_evt

OBJS_libsocket_evt := socket.o socket_afux.o
CARGS_socket := -I$(BOOLEAN) -I$(TIMEVAL) -I$(WAIT_EVT)
CARGS_socket_afux := -I$(BOOLEAN)

include $(TEMPLATES)/c.mk

