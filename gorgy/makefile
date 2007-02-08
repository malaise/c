include $(HOME)/Makefiles/common.mk

DINCL := boolean
DLIBA := timeval socket mutex sig_util

LIBS := libtty libudp
OBJS_libtty := tty.o
OBJS_libudp := udp.o

EXES := serial_spy gorgy
CARGS_serial_spy := -DSTANDARD
LIBS_serial_spy := libtty.a
CARGS_gorgy := -DSTANDARD
LIBS_gorgy := libtty.a libudp.a

include $(TEMPLATES)/c.mk

