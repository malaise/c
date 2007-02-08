#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define LP_NAME "/dev/lp0"

int main (void) {
  const char form_feed = (char) 0x0C;
  FILE * lp;
  
  lp = fopen (LP_NAME, "a");
  if (lp == (FILE*)NULL) {
    perror ("fopen");
    fprintf (stderr, "Cannot open stream %s\n", LP_NAME);
    exit (1);
  }

  (void) fputc ((int)form_feed, lp);
  (void) fclose (lp);
  exit (0);
}

