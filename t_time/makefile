include $(HOME)/Makefiles/common.mk

BOOLEAN=../boolean
TIMEVAL=../timeval

EXES := t_time
CARGS_t_time := -I$(BOOLEAN) -I$(TIMEVAL)
LARGS_t_time := $(TIMEVAL)/$(LIB)/libtimeval.a -lm

include $(TEMPLATES)/c.mk

