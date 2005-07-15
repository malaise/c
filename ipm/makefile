include $(HOME)/Makefiles/common.mk

SOCKET := ../socket
MUTEX=../mutex

EXES := tipm
CARGS_tipm := -I$(SOCKET)
LARGS_tipm := $(SOCKET)/$(LIB)/libsocket.a $(MUTEX)/$(LIB)/libmutex.a -lpthread

include $(TEMPLATES)/c.mk

