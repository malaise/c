#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define ERROR {printf ("ERROR. Usage %s [ -l | -U ] [ <msg> ]\n", argv[0]);\
               exit (1); }

int main (int argc, char *argv[]) {
int i, j;
char rep[256];
char *msg;
int to_up, to_lo;

    to_up = 0;
    to_lo = 0;
    msg = NULL;
    if (argc == 1) {
      ;
    } else if (argc == 2) {
      if (strcmp(argv[1], "-l") == 0) {
        to_lo = 1;
      } else if (strcmp(argv[1], "-U") == 0) {
        to_up = 1;
      } else {
        msg = argv[1];
      }
    } else if (argc == 3) {
      msg = argv[2];
      if (strcmp(argv[1], "-l") == 0) {
        to_lo = 1;
      } else if (strcmp(argv[1], "-U") == 0) {
        to_up = 1;
      } else {
        ERROR;
      }
    } else {
      ERROR;
    }

    if (msg != NULL) {
      printf ("%s", msg);
    }

    i = 0;
    for (;;) {
        rep[i] = (char) getchar();
        if (rep[i] == '\n') break;
        i++;
    }


    if (i == 0) {
      i = 1;
      rep[0] = '-';
    }
    rep[i] = '\0';

    for (j = 0; j < i; j++) {
        if (to_lo) {
            if ( (rep[j] >= 'A') && (rep[j] <= 'Z') ) {
                rep[j] = rep[j] - 'A' + 'a';
            }
        } else if (to_up) {
            if ( (rep[j] >= 'a') && (rep[j] <= 'z') ) {
                rep[j] = rep[j] - 'a' + 'A';
            }
        }
    }

    printf ("%s\n", rep);
    exit(0);
}

