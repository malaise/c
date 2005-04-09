#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <libgen.h>
#include <string.h>

#include "rusage.h"

#define L_MAX       10000
#define N_MAX       1000
#define MALLOC_MAX (100 * 1024)

/* Random value from 1 to max included */
static int rnd (int max) {
    return ((double)rand() / ( (double)RAND_MAX + 1.0) * (double)max) + 1;
}

int main (int argc, char *argv[]) {

  unsigned int seed;
  unsigned int n, i, j, size;
  void * array[N_MAX];

  /* Fixed seed */
  seed = 0;
  if (argc == 2) {
    seed = atoi (argv[1]);
  }
  if (seed < 0) seed = 0;
  srand (seed);

  printf ("%s pid is %d\n", basename(argv[0]), getpid());


  if (init_rusage() != RUSAGE_OK) {
    perror ("init_rusage");
    exit (1);
  }
  dump_rusage_str ("Start");

  for (i = 1; i <= L_MAX; i++) {
    n = rnd (N_MAX);

    /* N mallocs of random size */
    size = rnd (MALLOC_MAX);
    for (j = 0; j < n; j++) {
      array[j] = malloc (size);
      if (array[j] == NULL) {
        perror ("malloc");
        exit (1);
      }
    }

    /* Free */
    for (j = 0; j < n; j++) {
      free (array[j]);
    }
  }

  dump_rusage_str ("Stop");
  printf ("Done\n");
  exit (0);
}


