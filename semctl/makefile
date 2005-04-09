include $(HOME)/Makefiles/common.mk

SEM_UTIL=../sem_util

CCOPT=-O $(CCOPT)

EXES := semget semctl
CARGS_semget := -I$(SEM_UTIL)
LARGS_semget := $(SEM_UTIL)/$(LIB)/libsem_util.a -ltimeval -lm

include $(TEMPLATES)/c.mk


