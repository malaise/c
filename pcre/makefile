include $(HOME)/Makefiles/common.mk

EXES := tpcre tpcre2 tposix2pcre2

# PCRE OK
LARGS_tpcre := -lpcreposix -lpcre

# PCRE2 NOK
# On Ubuntu, erroneously links to the standard POSIX library instead of
# pcre2-posix, which leads tpcre2 to core in regfree after success, and to
# compile OK erroneous regex 't(', then regexec fails on error "No match"
LARGS_tpcre2 := -lpcre2-posix -lpcre2-8

# POSIX2PCRE OK
LARGS_tposix2pcre2 := -lposix2pcre -lpcre2-posix -lpcre2-8

include $(TEMPLATES)/c.mk

