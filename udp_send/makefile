include $(HOME)/Makefiles/common.mk

SOCKET=../socket

EXES := udp_send

CARGS_udp_send := -I$(SOCKET)
LARGS_udp_send := $(SOCKET)/$(LIB)/libsocket.a -ltimeval

include $(TEMPLATES)/c.mk

