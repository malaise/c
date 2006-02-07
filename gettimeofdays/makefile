include $(HOME)/Makefiles/common.mk

BOOLEAN=../boolean
TIMEVAL=../timeval
SEM_UTIL=../sem_util
RUSAGE=../rusage

EXES := gettimeofdays

CARGS_gettimeofdays := -I$(BOOLEAN) -I$(TIMEVAL) -I$(SEM_UTIL) -I$(RUSAGE)
LARGS_gettimeofdays := $(SEM_UTIL)/$(LIB)/libsem_util.a \
$(RUSAGE)/$(LIB)/librusage.a $(TIMEVAL)/$(LIB)/libtimeval.a -lm

include $(TEMPLATES)/c.mk

