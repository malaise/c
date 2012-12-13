include $(HOME)/Makefiles/common.mk

DLIBA := rusage

LIBS := lib_r
EXES := mallocer_m mallocer_h mallocer_r

OBJS_lib_r := lib

LIBS_mallocer_m :=
LIBS_mallocer_h := lib_r
LIBS_mallocer_r := lib_r

include $(TEMPLATES)/c.mk

$(BIN)/mallocer_m $(BIN)/mallocer_h $(BIN)/mallocer_r : $(LIB)/mallocer.o
	@if [ "$(LIBS_$(@F))" != "" ]; then \
	  make $(patsubst %,$(LIB)/%.a,$(LIBS_$(@F))); \
	fi
	@$(ECHO) CC -o $@ $< $(LIBS_$(@F):%=$(LIB)/%.a) $(DLIBAD) $(LARGS_$(@F)) -lpthread -lm
	@$(CC) -o $@ $< $(LIBS_$(@F):%=$(LIB)/%.a) $(DLIBAD) $(LARGS_$(@F)) -lpthread -lm

