include $(HOME)/Makefiles/common.mk

BOOLEAN := ../boolean
SOCKET := ../socket
MUTEX=../mutex

EXES := tipm
CARGS_tipm := -I$(BOOLEAN) -I$(SOCKET)
LARGS_tipm := $(SOCKET)/$(LIB)/libsocket.a $(MUTEX)/$(LIB)/libmutex.a -lpthread

include $(TEMPLATES)/c.mk

