include $(HOME)/Makefiles/common.mk

SIG_UTIL=../sig_util

EXES := shm
#EXES := tshm

CARGS_shm := -I$(SIG_UTIL)
LARGS_shm := $(SIG_UTIL)/$(LIB)/libsig_util.a
CARGS_tshm := -I$(SIG_UTIL)
LARGS_tshm := $(SIG_UTIL)/$(LIB)/libsig_util.a

include $(TEMPLATES)/c.mk

