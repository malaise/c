include $(HOME)/Makefiles/common.mk

SEM_UTIL=../sem_util
RUSAGE=../rusage

EXES := gettimeofdays

CARGS_gettimeofdays := -I$(SEM_UTIL) -I$(RUSAGE)
LARGS_gettimeofdays := $(SEM_UTIL)/$(LIB)/libsem_util.a $(RUSAGE)/$(LIB)/librusage.a -ltimeval -lm

include $(TEMPLATES)/c.mk

