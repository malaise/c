#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "rusage.h"

int main (void) {

  int i;
  void __attribute__((unused)) *p;

  if (init_rusage() != RUSAGE_OK) {
    fprintf (stderr, "Cannot init rusage\n");
    exit (1);
  }


  for (;;)  {
     dump_rusage_str ("Call");
     for (i=1; i< 10000; i++)
        p = malloc(1 * 1024 * 1024);
     (void) sleep (1) ;
  }

}

