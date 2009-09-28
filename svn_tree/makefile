include $(HOME)/Makefiles/common.mk

# SVN and APACHE includes and libs
SVN := /usr/local
APACHE := /usr

SVN_INC := $(SVN)/include/subversion-1
SVN_LIB := $(SVN)/lib

APACHE_INC := $(APACHE)/include/apr-1.0
APACHE_LIB := $(APACHE)/lib

# Local
DINCL := boolean
DLIBA := dynlist

LIBS := util
EXES := svn_tree

OBJS_libutil := callbacks.o utilities.o

CFLAGS = -I$(APACHE_INC) -I$(SVN_INC)

LARGS_svn_tree := -L$(SVN_LIB) -lsvn_client-1 -L$(APACHE_LIB) -lapr-1
LIBS_svn_tree := libutil.a

include $(TEMPLATES)/c.mk

CCOPT := $(CCOPT:-Werror=)

