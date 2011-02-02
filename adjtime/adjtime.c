#include <stdio.h>
#include <stdlib.h>

#include "adjtime_call.h"

int main(int argc, char *argv[]) {

  struct timeval delta, old_delta, *p_delta;

  float printed_delta;

  if (argc > 2) {
    fprintf (stderr, "Syntax error. Usage : adjtime [ <secs> ]\n");
    exit (1);
  }

  if (argc == 2) {
    p_delta = &delta;
    sscanf (argv[1], "%f", &printed_delta);
    delta.tv_sec = (time_t)printed_delta;
    delta.tv_usec = (printed_delta - (float) delta.tv_sec) * 1000000;
    printf ("adjtime (%d.%06d)\n", (int)delta.tv_sec, abs((int)delta.tv_usec));
  } else {
    p_delta = (struct timeval*) NULL;
  }

  if (adjtime_call (p_delta, &old_delta) != 0) {
    exit (1);
  } else {
    printed_delta = old_delta.tv_sec + (float) old_delta.tv_usec / 1000000.0;
    printf ("remaining (%3.06f)\n", printed_delta);
  }

  exit (0);

}
