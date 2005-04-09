include $(HOME)/Makefiles/common.mk

LIBS := librusage
OBJS_librusage := rusage.o

EXES := anal_rusage t_rusage
LIBS_anal_rusage := librusage.a
LIBS_t_rusage := librusage.a

include $(TEMPLATES)/c.mk

$(LIB)/librusage.a $(LIB)/librusage.so : $(OBJS_librusage:%=$(LIB)/%)


rusage.c : rusage.h
anal_rusage.c : rusage.h
t_rusage.c : rusage.h

