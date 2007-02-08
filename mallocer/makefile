include $(HOME)/Makefiles/common.mk

DLIBA := rusage

LIBS := lib_r
EXES := mallocer_m mallocer_h mallocer_r

OBJS_lib_r := lib.o


LIBS_mallocer_m :=
LIBS_mallocer_h := lib_r.so
LIBS_mallocer_r := lib_r.so

include $(TEMPLATES)/c.mk

$(BIN)/mallocer_m $(BIN)/mallocer_h $(BIN)/mallocer_r : $(LIB)/mallocer.o
	@if [ "$(LIBS_$(@F))" != "" ]; then \
	  make $(patsubst %,$(LIB)/%,$(LIBS_$(@F))); \
	fi
	$(CC) -o $@ $< $(LIBS_$(@F):%=$(LIB)/%) $(DLIBAD) $(LARGS_$(@F)) -lpthread -lm

