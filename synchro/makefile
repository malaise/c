include $(HOME)/Makefiles/common.mk

GORGY := ../gorgy
ADJTIME := ../adjtime

DINCL := boolean
DLIBA := timeval socket mutex sig_util wait_evt

LIBS := gorgy_decode
EXES := synchro_server synchro_client synchro_serial eti

OBJS_gorgy_decode := gorgy_decode.o

LIBS_synchro_serial := gorgy_decode.a
CARGS_synchro_serial := -I$(GORGY)
LARGS_synchro_serial := $(GORGY)/$(LIB)/libtty.a

LIBS_eti := gorgy_decode.a
CARGS_eti := -I$(GORGY) -I$(ADJTIME)
LARGS_eti := $(GORGY)/$(LIB)/libtty.a $(ADJTIME)/$(LIB)/adjtime_call.a

include $(TEMPLATES)/c.mk

