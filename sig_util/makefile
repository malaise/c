include $(HOME)/Makefiles/common.mk

DINCL := boolean
DLIBA := timeval

LIBS := libsig_util
EXES := t_pause t_sig

OBJS_libsig_util := sig_util.o

LIBS_t_pause := libsig_util.a

LIBS_t_sig := libsig_util.a
LARGS_t_sig := -lcutil -lm

include $(TEMPLATES)/c.mk

