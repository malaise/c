#include "stdio.h"
#include "stdlib.h"

#define usage(n) { fprintf (stderr, "Usage : fread_float <file_name> <offset_in_bytes> [ <number> ] [ -d ]\n"); \
                   exit (n); }

int main (int argc, char *argv[]) {
  int number = 1;
  int double_float = 0;
  long int offset;
  int i;
  float f;
  double d;
  int res;

  FILE *file;

  if (argc == 4) {
    if (strcmp(argv[3], "-d") == 0) {
      double_float = 1;
    } else {
      number = atoi(argv[3]);
    }
  } else if (argc == 5) {
    if (strcmp(argv[4], "-d") == 0) {
      double_float = 1;
      number = atoi(argv[3]);
    } else {
      usage(1);
    }
  } else if (argc != 3) {
    usage(1);
  }
  if (number == 0) {
    usage(1);
  }
  offset = atol(argv[2]);
  if (offset == 0) {
    usage(1);
  }

  printf ("Reading %d %s at pos %ld in file %s\n",
           number, (double_float ? "double(s)" : "float(s)"), offset, argv[1]);

  file = fopen (argv[1], "r");
  if (file == (FILE*)NULL) {
    perror ("fopen");
    fprintf (stderr, "cannot open file %s\n", argv[1]);
    usage(2);
  }

  if (fseek (file, offset-1, SEEK_SET) == -1) {
    perror ("fseek");
    fprintf (stderr, "cannot move at pos %ld in file %s\n", offset, argv[1]);
    usage(2);
  }

  for (i = 1; i <= number; i++) {
    if (double_float) {
      res = fread (&d, sizeof(double), 1, file);
    } else {
      res = fread (&f, sizeof(float), 1, file);
    }
    if (res != 1) {
      perror ("fread");
      fprintf (stderr, "cannot read %d %s at pos %ld in file %s\n",
               number, (double_float ? "double(s)" : "float(s)"), offset, argv[1]);
      usage(2);
    }

    if (double_float) {
      printf ("%g\n", d);
    } else {
      printf ("%f\n", f);
    }
  }

  exit (0);

}
