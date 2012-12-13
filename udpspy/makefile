include $(HOME)/Makefiles/common.mk

DINCL=boolean
DLIBA=socket_evt timeval wait_evt


EXES := udpspy
LIBS := libutil

OBJS_libutil := util

LIBS_udpspy := libutil
LARGS_udpspy := -lpthread -lm

include $(TEMPLATES)/c.mk

