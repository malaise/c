include $(HOME)/Makefiles/common.mk

RUSAGE = ../rusage

LIBS := lib_r
EXES := mallocer_m mallocer_h mallocer_r

OBJS_lib_r := lib.o

CARGS_mallocer := -I$(RUSAGE)

LARGS_mallocer_m := $(RUSAGE)/lib_Linux/librusage.a -lpthread
LIBS_mallocer_m :=
LARGS_mallocer_h := $(RUSAGE)/lib_Linux/librusage.a -lpthread
LIBS_mallocer_h := lib_r.so
LARGS_mallocer_r := $(RUSAGE)/lib_Linux/librusage.a -lpthread
LIBS_mallocer_r := lib_r.so

include $(TEMPLATES)/c.mk

$(BIN)/mallocer_m $(BIN)/mallocer_h $(BIN)/mallocer_r : $(LIB)/mallocer.o
	@if [ "$(LIBS_$(@F))" != "" ]; then \
	  make $(patsubst %,$(LIB)/%,$(LIBS_$(@F))); \
	fi
	$(CC) -o $@ $< $(LIBS_$(@F):%=$(LIB)/%) $(LARGS_$(@F))

