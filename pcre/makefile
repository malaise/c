include $(HOME)/Makefiles/common.mk

EXES := tpcre tpcre2

# OK
LARGS_tpcre := -lpcreposix -lpcre
# Erroneously links to the standard POSIX library instead of pcre2-posix,
# which leads tpcre2 to core in regfree after success, and to compile OK
# erroneous regex 't(' then regexec to fail on error "No match"
LARGS_tpcre2 := -lpcre2-posix -lpcre2-8

include $(TEMPLATES)/c.mk

