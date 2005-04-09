#include <stdio.h>
#include <stdlib.h>

#define LEN 50

int main (int argc, char *argv[]) {

  short s, *ps;
  char str[LEN];
  int i;
  int v;

  if (argc == 2) {
    v = atoi(argv[1]);
  } else {
    exit (1);
  }

  for (i = 0; i < LEN; i++) {
    str[i] = (char) v;
  }

  printf ("%d : ", v);
  for (i = 0; i < 4 ; i++) {
    ps = (short*) (&str[i]);
    s = *ps;
    printf ("%d -> %04hX    ", i, s);
  }
  printf ("\n");
  exit(0);
}
  
