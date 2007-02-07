include $(HOME)/Makefiles/common.mk

BOOLEAN := ../boolean
MUTEX := ../mutex
SOCKET=../socket
GET_LINE=../get_line

EXES := udp_send

CARGS_udp_send := -I$(BOOLEAN) -I$(SOCKET) -I$(GET_LINE)
LARGS_udp_send := $(SOCKET)/$(LIB)/libsocket.a $(MUTEX)/$(LIB)/libmutex.a \
  $(GET_LINE)/$(LIB)/libget_line.a -lpthread

include $(TEMPLATES)/c.mk

