#include <time.h>
#include <stdio.h>
#include <unistd.h>

int main (void) {

  time_t t1, t2;
  struct tm *ptm;

  t1 = time(&t2);

  printf ("time %d %d\n", (int)t1, (int)t2);

  ptm = localtime(&t1);

  printf ("localtime %d:%d\n", ptm->tm_hour, ptm->tm_min);

  exit (0);
}
