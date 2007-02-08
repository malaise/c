#ifndef __GET_LINE__
#define __GET_LINE__

/* INCLUDE FILES */
/*****************/
#include <stdio.h>

#define NUL '\0'
#define EOL '\n'

/* Reads at most len chars from FILE (stdin if FILE=NULL) */
/*  or until EOL (return) is entered. */
/* The out string is always NUL terminated */
/* Returns the strlen of str if OK */
/* It returns -1 if EOF or error */
extern int get_line (FILE *file, char *str, const int len);

/* Reads at most len chars from FILE (stdin if FILE=NULL) */
/*  or until EOL (return) is entered. */
/* The out string is always NUL terminated */
/* Returns the strlen of str */
/* Reads characters up to a New_Line (that is appended) */
/*  or up to the end of file. */
/*  So, either the returned string ends with a New_Line and */
/*   another get can be performed, */
/*  Or the string does not end with New_Line (or is empty) and */
/*   the end of file has been reached or an error has occured. */
extern unsigned int get_text (FILE *file, char *str,
                              const unsigned int len);

#endif /* _GET_LINE */

