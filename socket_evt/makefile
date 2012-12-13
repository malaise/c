include $(HOME)/Makefiles/common.mk

DINCL := boolean
DLIBA := timeval wait_evt

FILES2LINK := socket.h socket_afux.h socket_prv.h socket_afux.c socket_net.h \
              socket.c
LINKFROM := $(HOME)/ada/c

LIBS := libsocket_evt

OBJS_libsocket_evt := socket socket_afux

include $(TEMPLATES)/c.mk

