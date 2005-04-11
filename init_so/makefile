include $(HOME)/Makefiles/common.mk

LIBS := libmodule1 libmodule2
OBJS_libmodule1 := module1.o
OBJS_libmodule2 := module2.o

EXES := try
LIBS_try := libmodule1.so libmodule2.so

include $(TEMPLATES)/c.mk

try.c : module1.h module2.h
module1.c : module1.h
module2.c : module2.h

