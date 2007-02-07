#include <errno.h>

#include "get_line.h"

/* Reads at most len chars from FILE (stdin if FILE=NULL) */
/*  or until Cr (return) is entered. */
/* The out string is always NUL terminated (Cr removed) */
/* Returns the strlen of str if OK */
/* It returns -1 if EOF or error */
int get_line (file, str, len) 
    FILE *file;
    char str[];
    int len;
{
    int i;
    char c;

    errno = 0;

    if (len <= 0) return (0);

    str[0] = NUL;

    /* Read line */
    i = -1;
    do {
        i ++;
        if (i >= len - 1) {
          /* End of buffer */
          str[i] = NUL;
          break;
        } 
        /* Read char */
        if (file == (FILE*)NULL) {
            c = (char) getchar();
        } else {
            c = (char) fgetc(file);
        }
        if ((int)c == EOF) {
            /* Error or end of file */
            return (-1);
        } else if (c == EOL) {
            /* End of line */
            str[i] = NUL;
        } else {
            str[i] = c;
        }

    } while (str[i] != NUL);

       
    return (i);

}

