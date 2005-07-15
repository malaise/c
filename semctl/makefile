include $(HOME)/Makefiles/common.mk


BOOLEAN=../boolean
SEM_UTIL=../sem_util

CCOPT := -O $(CCOPT)

EXES := semget semctl
CARGS_semget := -I$(BOOLEAN) -I$(SEM_UTIL)
LARGS_semget := $(SEM_UTIL)/$(LIB)/libsem_util.a -lcutil -lm

include $(TEMPLATES)/c.mk


