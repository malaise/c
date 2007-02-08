#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "get_line.h"

static void remove_file(char *name) {
  if (unlink(name) == 0) {
    printf (">%s< deleted.\n", name);
  } else {
    printf ("Error deleting >%s<. Errno --> %d : %s\n", name, errno, strerror(errno));
  }
}

int main (int argc, char *argv[]) {

int i;
char input[500];

  if (argc == 1) {

    system ("ls");
    for (;;) {
      printf ("File name : ");
      i = get_line ( (FILE*)NULL, input, sizeof(input));
      if (i < 1) {
        break;
      }
      remove_file(input);
    }
      
  } else {

    for (i = 1; i < argc; i++) {
      for (;;) {
        printf ("OK to remove file >%s< ? (Y/N/Q) ", argv[i]);
        (void) get_line ((FILE*)NULL, input, sizeof(input));
        if ( (strcmp(input, "y") == 0) || (strcmp(input, "Y") == 0) ) {
          remove_file(argv[i]);
          break;
        } else if ( (strcmp(input, "n") == 0) || (strcmp(input, "N") == 0) ) {
          break;
        } else if ( (strcmp(input, "q") == 0) || (strcmp(input, "Q") == 0) ) {
          printf ("Aborted.\n");
          exit(0);
        }
      } /* for (;;) */
      printf ("\n");
    } /* for each arg */

  }
  printf ("Done.\n");
  exit(0);
}
