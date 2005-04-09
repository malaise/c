include $(HOME)/Makefiles/common.mk

LIBS := libsig_util
EXES := t_pause t_sig

OBJS_libsig_util := sig_util.o


LIBS_t_pause := libsig_util.a

CARGS_t_sig :=
LIBS_t_sig := libsig_util.a
LARGS_t_sig := -ltimeval -lm
LIBS_t_sig := libsig_util.a 

include $(TEMPLATES)/c.mk

$(LIB)/libsig_util.a $(LIB)/libsig_util.so : $(OBJS_libsig_util:%=$(LIB)/%)

sig_util.c : sig_util.h
t_pause.c : sig_util.h
t_sig.c : sig_util.h

