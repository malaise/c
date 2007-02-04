include $(HOME)/Makefiles/common.mk

# -Wunreach fails on last loop
CCOPT_Linux     ?= -pedantic -Wall -W -Wpointer-arith \
  -Wbad-function-cast -Wcast-qual -Wcast-align -Wwrite-strings \
  -Wsign-compare -Wstrict-prototypes -Wmissing-prototypes  \
  -Wmissing-declarations -Wmissing-noreturn -Winline \
  -Wfloat-equal -Wundef

EXES := tcpdump

include $(TEMPLATES)/c.mk

