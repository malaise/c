include $(HOME)/Makefiles/common.mk

BOOLEAN=../boolean
TIMEVAL=../timeval

EXES := delay

CARGS_delay := -I$(BOOLEAN) -I$(TIMEVAL)
LARGS_delay := $(TIMEVAL)/$(LIB)/libtimeval.a -lm

include $(TEMPLATES)/c.mk

