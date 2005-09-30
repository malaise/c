#include <stdio.h>
#include <stdlib.h>
#include "timeval.h"


int main(void) {
timeout_t orig, new;

  get_time (&new);

  do {
    get_time (&orig);
  } while (comp_time (&orig, &new) == 0);

  do {
    get_time (&new);
  } while (comp_time (&orig, &new) == 0);

  (void) sub_time (&new, &orig);

  printf ("Tick : %ld sec . %06ld usec \n", new.tv_sec, new.tv_usec);
  exit(0);
}

