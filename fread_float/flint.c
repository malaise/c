#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define usage(n) { \
  fprintf (stderr, "Usage : flint <float> [ -d ]\n"); \
  fprintf (stderr, "   or : flint <hex> <hex> <hex> <hex> [ <hex> <hex> <hex> <hex> ]\n"); \
  exit (n); \
}

int main (int argc, char *argv[]) {

  double d;

  double *dp = &d;
  float  *fp = (float*)&d;
  unsigned char *bp = (unsigned char *)&d;
  int n, i;
  unsigned int u;


  if (argc == 2) {
    sscanf (argv[1], "%f", fp);
    n = 4;
  } else if (argc == 3) {
    if (strcmp(argv[2], "-d") != 0) {
      usage(1);
    }
    sscanf (argv[1], "%lf", dp);
    n = 8;
  } else if ( (argc == 5) || (argc == 9) ) {
    n = argc - 1;
    for (i = 1; i <= n; i++, bp++) {
      sscanf (argv[i], "%x", &u);
      *bp = (unsigned char) u;
    }

  } else {
    usage(1);
  }

  if (argc <= 3) {
    for (i = 1; i <= n; i++, bp++) {
      printf ("%02X ", (int)*bp);
    }
    printf ("\n");
  } else if (argc == 5) {
    printf ("%f\n", *fp);
  } else {
    printf ("%g\n", *dp);
  }

  exit (0);

}




    
