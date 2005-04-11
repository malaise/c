include $(HOME)/Makefiles/common.mk

LIBS := libcircul
OBJS_libcircul := circul.o 

EXES := cirtail uncircular
LIBS_cirtail := libcircul.a
LARGS_cirtail := -lm
LIBS_uncircular := libcircul.a
LARGS_uncircular := -lm

include $(TEMPLATES)/c.mk

cirtail.c : circul.h
uncircular.c : circul.h
circul.c : circul.h

