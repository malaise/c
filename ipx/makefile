include $(HOME)/Makefiles/common.mk

BOOLEAN := ../boolean
SOCKET := ../socket
MUTEX := ../mutex

EXES := ipx
CARGS_ipx := -I$(BOOLEAN) -I$(SOCKET)
LARGS_ipx := $(SOCKET)/$(LIB)/libsocket.a $(MUTEX)/$(LIB)/libmutex.a -lpthread

include $(TEMPLATES)/c.mk

