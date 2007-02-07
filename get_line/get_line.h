#ifndef	__GET_LINE__
#define __GET_LINE__

/* INCLUDE FILES */
/*****************/
#include <stdio.h>

#ifdef __STDC__
#define ARGS(x) x
#else
#define ARGS(x) ()
#endif

#define NUL '\0'
#define EOL '\n'

/* Reads at most len chars from FILE (stdin if FILE=NULL) */
/*  or until EOL (return) is entered. */
/* The out string is always NUL terminated */
/* Returns the strlen of str if OK */
/* It returns -1 if EOF or error */
extern int get_line 
       ARGS(( FILE *file,
              char *str,
              int len));

#endif	/* _GET_LINE */
