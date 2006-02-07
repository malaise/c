include $(HOME)/Makefiles/common.mk

BOOLEAN=../boolean
TIMEVAL=../timeval
GORGY=../gorgy
MUTEX := ../mutex
SOCKET=../socket
SIG_UTIL=../sig_util
ADJTIME=../adjtime
WAIT_EVT=../wait_evt

LIBS := gorgy_decode
EXES := synchro_server synchro_client synchro_serial eti

OBJS_gorgy_decode := gorgy_decode.o

CARGS_synchro_server := -I$(BOOLEAN) -I$(TIMEVAL) -I$(SOCKET)
LARGS_synchro_server := $(SOCKET)/$(LIB)/libsocket.a $(MUTEX)/$(LIB)/libmutex.a $(TIMEVAL)/$(LIB)/libtimeval.a -lpthread -lm
CARGS_synchro_client := -I$(BOOLEAN) -I$(TIMEVAL) -I$(SOCKET) -I$(WAIT_EVT)
LARGS_synchro_client := $(SOCKET)/$(LIB)/libsocket.a $(MUTEX)/$(LIB)/libmutex.a $(WAIT_EVT)/$(LIB)/libwait_evt.a $(TIMEVAL)/$(LIB)/libtimeval.a -lpthread -lm
CARGS_synchro_serial := -I$(BOOLEAN) -I$(TIMEVAL) -I$(SIG_UTIL) -I$(GORGY)
LARGS_synchro_serial := $(SIG_UTIL)/$(LIB)/libsig_util.a $(GORGY)/$(LIB)/libtty.a $(TIMEVAL)/$(LIB)/libtimeval.a -lm
LIBS_synchro_serial := gorgy_decode.a
CARGS_eti := -I$(BOOLEAN) -I$(TIMEVAL) -I$(SIG_UTIL) -I$(GORGY) -I$(ADJTIME)
LARGS_eti := $(SIG_UTIL)/$(LIB)/libsig_util.a $(GORGY)/$(LIB)/libtty.a $(ADJTIME)/$(LIB)/adjtime_call.a $(TIMEVAL)/$(LIB)/libtimeval.a -lm
LIBS_eti := gorgy_decode.a

include $(TEMPLATES)/c.mk

