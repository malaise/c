include $(HOME)/Makefiles/common.mk

LIBS := libcircul
OBJS_libcircul := circul.o 

EXES := cirtail uncircular
LIBS_cirtail := libcircul.a
LARGS_cirtail := -lm
LIBS_uncircular := libcircul.a
LARGS_uncircular := -lm

include $(TEMPLATES)/c.mk

$(LIB)/libcircul.a $(LIB)/libcircul.so : $(OBJS_libcircul:%=$(LIB)/%)

cirtail.c : circul.h
uncircular.c : circul.h
circul.c : circul.h

