include $(HOME)/Makefiles/common.mk

BOOLEAN := ../boolean
TIMEVAL := ../timeval

FILES2LINK := wait_evt.h wait_evt.c
LINKFROM := $(HOME)/ada/c

LIBS := libwait_evt

OBJS_libwait_evt := wait_evt.o
CARGS_wait_evt := -I$(BOOLEAN) -I$(TIMEVAL)

include $(TEMPLATES)/c.mk

