include $(HOME)/Makefiles/common.mk

SOCKET := ../socket

EXES := tipm
CARGS_tipm := -I$(SOCKET)
LARGS_tipm := $(SOCKET)/$(LIB)/libsocket.a

include $(TEMPLATES)/c.mk

