include $(HOME)/Makefiles/common.mk

GORGY=../gorgy
SOCKET=../socket
SIG_UTIL=../sig_util
ADJTIME=../adjtime
WAIT_EVT=../wait_evt

LIBS := gorgy_decode
EXES := synchro_server synchro_client synchro_serial eti

OBJS_gorgy_decode := gorgy_decode.o

CARGS_synchro_server := -I$(SOCKET)
LARGS_synchro_server := $(SOCKET)/$(LIB)/libsocket.a -ltimeval
CARGS_synchro_client := -I$(SOCKET) -I$(WAIT_EVT)
LARGS_synchro_client := $(SOCKET)/$(LIB)/libsocket.a $(WAIT_EVT)/$(LIB)/libwait_evt.a -ltimeval
CARGS_synchro_serial := -I$(SIG_UTIL) -I$(GORGY)
LARGS_synchro_serial := $(SIG_UTIL)/$(LIB)/libsig_util.a $(GORGY)/$(LIB)/libtty.a -ltimeval
LIBS_synchro_serial := gorgy_decode.a
CARGS_eti := -I$(SIG_UTIL) -I$(GORGY) -I$(ADJTIME)
LARGS_eti := $(SIG_UTIL)/$(LIB)/libsig_util.a $(GORGY)/$(LIB)/libtty.a $(ADJTIME)/$(LIB)/adjtime_call.a -ltimeval
LIBS_eti := gorgy_decode.a

include $(TEMPLATES)/c.mk

$(LIB)/gorgy_decode.a $(LIB)/gorgy_decode.so : $(OBJS_gorgy_decode:%=$(LIB)/%)

eti.c : gorgy_decode.h
gorgy_decode.c : gorgy_decode.h
synchro_client.c : synchro.h
synchro_serial.c : gorgy_decode.h
synchro_server.c : synchro.h
