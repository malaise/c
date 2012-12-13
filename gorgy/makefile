include $(HOME)/Makefiles/common.mk

DINCL := boolean
DLIBA := timeval sig_util socket mutex

LIBS := libtty libudp
OBJS_libtty := tty
OBJS_libudp := udp

EXES := serial_spy gorgy
CARGS_serial_spy := -DSTANDARD
LIBS_serial_spy := libtty
CARGS_gorgy := -DSTANDARD
LIBS_gorgy := libtty libudp

include $(TEMPLATES)/c.mk

