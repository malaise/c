#include <stdio.h>

int main(void) {
  FILE *file;
  float f = 21.0;
  double d = 22.0;

  file = fopen("toto", "w");
  fwrite (&f, sizeof(float), 1, file);
  fwrite (&d, sizeof(double), 1, file);
  return 0;
}

