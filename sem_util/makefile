include $(HOME)/Makefiles/common.mk

BOOLEAN=../boolean

CCOPT := -O -D$(HOST) $(CCOPT)

LIBS := libsem_util
OBJS_libsem_util := sem_util.o
CARGS_sem_util := -I$(BOOLEAN)

include $(TEMPLATES)/c.mk

