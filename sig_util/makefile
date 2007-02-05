include $(HOME)/Makefiles/common.mk

BOOLEAN=../boolean
TIMEVAL=../timeval

LIBS := libsig_util
EXES := t_pause t_sig

OBJS_libsig_util := sig_util.o

CARGS_sig_util := -I$(BOOLEAN)

CARGS_t_pause := -I$(BOOLEAN)
LIBS_t_pause := libsig_util.a

CARGS_t_sig := -I$(BOOLEAN) -I$(TIMEVAL)
LIBS_t_sig := libsig_util.a
LARGS_t_sig := -lcutil -lm

include $(TEMPLATES)/c.mk

