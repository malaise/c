include $(HOME)/Makefiles/common.mk

BOOLEAN=../boolean
TIMEVAL=../timeval

LIBS := libsig_util
EXES := t_pause t_sig

OBJS_libsig_util := sig_util.o


LIBS_t_pause := libsig_util.a

CARGS_t_sig := -I$(BOOLEAN) -I$(TIMEVAL)
LIBS_t_sig := libsig_util.a
LARGS_t_sig := $(TIMEVAL)/$(LIB)/libtimeval.a -lm
LIBS_t_sig := libsig_util.a 

include $(TEMPLATES)/c.mk

