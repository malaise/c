include $(HOME)/Makefiles/common.mk

SOCKET=../socket

EXES := time_spy

CARGS_time_spy := -I$(SOCKET)
LARGS_time_spy := $(SOCKET)/$(LIB)/libsocket.a -ltimeval

include $(TEMPLATES)/c.mk

