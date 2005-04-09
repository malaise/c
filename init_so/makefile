include $(HOME)/Makefiles/common.mk

LIBS := libmodule1 libmodule2
OBJS_libmodule1 := module1.o
OBJS_libmodule2 := module2.o

EXES := try
LIBS_try := libmodule1.so libmodule2.so

include $(TEMPLATES)/c.mk

$(LIB)/libmodule1.a $(LIB)/libmodule1.so : $(OBJS_libmodule1:%=$(LIB)/%)
$(LIB)/libmodule2.a $(LIB)/libmodule2.so : $(OBJS_libmodule2:%=$(LIB)/%)

try.c : module1.h module2.h
module1.c : module1.h
module2.c : module2.h

