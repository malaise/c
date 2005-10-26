#include <stdio.h>
#include <stdlib.h>

int main (int argc, char *argv[]) {

  int i;
  char *p;


  for (i = 1; i < argc; i++) {
    p = getenv (argv[i]);
    if (p == NULL) {
      printf ("%s is not set\n", argv[i]);
    } else {
      printf  ("%s is set to %s\n", argv[i], p);
    }
  }

  exit (0);
}

