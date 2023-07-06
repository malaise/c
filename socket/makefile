# Compile socket without depend to wait_evt
#  (evt_fd_set always returns False)
include $(HOME)/Makefiles/common.mk

DINCL := boolean
DLIBA := mutex

FILES2LINK := socket.h socket_afux.h socket_prv.h socket_afux.c socket_net.h \
              socket.c
LINKFROM := $(HOME)/ada/c

LIBS := libsocket

OBJS_libsocket := socket socket_afux
CARGS_socket := -DSOCKET_NO_EVT -DSOCKET_MUTEX

include $(TEMPLATES)/c.mk

