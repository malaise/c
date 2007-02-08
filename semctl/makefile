include $(HOME)/Makefiles/common.mk
#-Wunreach fails ctype.h
CCOPT_Linux     ?= -pedantic -Wall -W -Wpointer-arith \
  -Wbad-function-cast -Wcast-qual -Wcast-align -Wwrite-strings \
  -Wsign-compare -Wstrict-prototypes -Wmissing-prototypes  \
  -Wmissing-declarations -Wmissing-noreturn -Winline \
  -Wfloat-equal -Wundef

DINCL := boolean
DLIBA := timeval sem_util

CCOPT := -O $(CCOPT)

EXES := semget semctl

include $(TEMPLATES)/c.mk

