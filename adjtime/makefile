include $(HOME)/Makefiles/common.mk

LIBS := adjtime_call
OBJS_adjtime_call := adjtime_call.o

EXES := adjtime prof_adjtime
LIBS_adjtime := adjtime_call.a
LIBS_prof_adjtime := adjtime_call.a

include $(TEMPLATES)/c.mk

adjtime_call.c : adjtime_call.h
adjtime.c : adjtime_call.h
prof_adjtime.c : adjtime_call.h

