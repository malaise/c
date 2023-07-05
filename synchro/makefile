include $(HOME)/Makefiles/common.mk

GORGY := gorgy
LIB_TTY := ../$(GORGY)/$(LIB)/libtty.a

DINCL := boolean $(GORGY)
DLIBA := adjtime timeval socket_evt mutex sig_util wait_evt

LIBS := gorgy_decode
EXES := synchro_server synchro_client synchro_serial eti

OBJS_gorgy_decode := gorgy_decode

LIBS_synchro_serial := gorgy_decode
LARGS_synchro_serial := $(LIB_TTY)

LIBS_eti := gorgy_decode
LARGS_eti := $(LIB_TTY)

include $(TEMPLATES)/c.mk

