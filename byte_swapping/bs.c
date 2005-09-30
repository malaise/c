#include <stdio.h>
#include <stdlib.h>
int main (void) {

  int n;
  int i;
  char *p;

  printf ("01 02 03 04\n");
  for (i = 1, p = (char*)&n; i <= 4; i++, p++) {
    *p = (char)i;
  }
  printf ("%08X\n", n);
  exit(0);
}
