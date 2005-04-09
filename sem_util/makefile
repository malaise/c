include $(HOME)/Makefiles/common.mk

CCOPT := -O -D$(HOST) $(CCOPT)

LIBS := libsem_util
OBJS_libsem_util := sem_util.o

include $(TEMPLATES)/c.mk

sem_util.c : sem_util.h

$(LIB)/libsem_util.a $(LIB)/libsem_util.so : $(OBJS_libsem_util:%=$(LIB)/%)

