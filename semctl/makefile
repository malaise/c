include $(HOME)/Makefiles/common.mk
#-Wunreach fails ctype.h
CCOPT_Linux     ?= -pedantic -Wall -W -Wpointer-arith \
	-Wbad-function-cast -Wcast-qual -Wcast-align -Wwrite-strings \
	-Wsign-compare -Wstrict-prototypes -Wmissing-prototypes  \
	-Wmissing-declarations -Wmissing-noreturn -Winline \
	-Wfloat-equal -Wundef

BOOLEAN=../boolean
TIMEVAL=../timeval
SEM_UTIL=../sem_util

CCOPT := -O $(CCOPT)

EXES := semget semctl
CARGS_semget := -I$(BOOLEAN) -I$(TIMEVAL) -I$(SEM_UTIL)
LARGS_semget := $(SEM_UTIL)/$(LIB)/libsem_util.a -lcutil -lm

include $(TEMPLATES)/c.mk


