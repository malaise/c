#include <stdio.h>
#include <stdlib.h>
#include "adjtime_call.h"

int main(void) {
  int i, j;
  struct timeval t1, t2;
  float printed_delta;

  for (i = 0; i < 10; i++) {
    t1.tv_sec = i;
    t1.tv_usec = 0;
    for (j=0; j<100; j++)
      if (adjtime_call(&t1, &t2) == -1)
        exit(0);
    printed_delta = t2.tv_sec + (float) t2.tv_usec / 1000000.0;
    printf ("remaining (%3.06f)\n", printed_delta);
  }


  for (i = -10; i <= 0; i++) {
    t1.tv_sec = i;
    t1.tv_usec = 0;
    for (j=0; j<100; j++)
      if (adjtime_call(&t1, &t2) == -1)
        exit(0);
    printed_delta = t2.tv_sec + (float) t2.tv_usec / 1000000.0;
    printf ("remaining (%3.06f)\n", printed_delta);
  }
  exit(0);
}
