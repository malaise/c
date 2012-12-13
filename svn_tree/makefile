include $(HOME)/Makefiles/common.mk

# SVN and APACHE includes and libs
SVN := /usr/local
APACHE=/usr/local/apr

SVN_INC := $(SVN)/include/subversion-1
SVN_LIB := $(SVN)/lib

APACHE_INC := $(APACHE)/include/apr-1
APACHE_LIB := $(APACHE)/lib

# Local
DINCL := boolean
DLIBA := dynlist

LIBS := util
EXES := svn_tree

OBJS_libutil := callbacks utilities

CFLAGS = -I$(APACHE_INC) -I$(SVN_INC)

LARGS_svn_tree := -L$(SVN_LIB) -lsvn_client-1 $(APACHE_LIB)/libapr-1.a
LIBS_svn_tree := libutil

include $(TEMPLATES)/c.mk

CCOPT := $(CCOPT:-pedantic=)

