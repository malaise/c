/* Removes all "<char> Backspace" sequences from stdin to stdout */
#include <stdio.h>
#include <stdlib.h>

int main (void) {

  int c1, c2;

    c1 = (char) getchar();
    if ((int)c1 == EOF)  exit (0);

  for (;;) {
    c2 = (char) getchar();

    if ( (int)c2 == 8 )  {
      /* A backspace. Read next char */
      c1 = (char) getchar();
    } else if ((int)c2 == EOF) {
      /* End of file. Done. */
      break;
    } else {
      /* Other char */
      putchar (c1);
      c1 = c2;
    }
  }
  exit(0);

}

