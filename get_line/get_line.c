#include <errno.h>

#include "get_line.h"

/* Reads at most len chars from FILE (stdin if FILE=NULL) */
/*  or until EOL (return) is entered. */
/* The out string is always terminated by "\n\0" */
/* Returns the strlen of str if OK */
/* It returns 0 if EOF or error */
extern unsigned int get_text (FILE *file, char *str,
                              const unsigned int len) {
    unsigned int u;
    char c;

    errno = 0;

    if (len <= 0) return (0);


    /* Read line */
    u = 0;
    do {
        if (u >= len - 1) {
          /* End of buffer */
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
            break;
        } else {
            str[u] = c;
            u++;
        }

    } while (c != EOL);

    str[u] = NUL;
    return (u);
}

/* Reads at most len chars from FILE (stdin if FILE=NULL) */
/*  or until Cr (return) is entered. */
/* The out string is always NUL terminated (Cr removed) */
/* Returns the strlen of str if OK */
/* It returns -1 if EOF or error */
int get_line (FILE *file, char *str, const int len) {

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

