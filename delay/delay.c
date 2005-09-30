#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "boolean.h"
#include "timeval.h"

extern char *basename (const char *filename);

int main(int argc, char *argv[]) {

  timeout_t timeout;
  int i, j;

  timeout.tv_sec = 0;
  timeout.tv_usec = 0;

  if (argc == 1) {
    exit (0);
  } else if (argc > 3) {
    fprintf (stderr, "Syntax Error. Usage : %s  [secs [msecs]]\n", basename(argv[0]));
    exit (1);
  } else {
    for (i = 1; i < argc; i++) {
      for (j = 0; (unsigned int)j < strlen(argv[i]); j++) {
        if (! isdigit((int)argv[i][j])) {
          fprintf (stderr, "Syntax Error. Usage : %s  [secs [msecs]]\n", basename(argv[0]));
          exit (1);
        }
      }
    }
  }


  timeout.tv_sec = atoi(argv[1]);

  if (argc == 3) timeout.tv_usec = atoi(argv[2]) * 1000;

  delay(&timeout);

  exit (0);

}
