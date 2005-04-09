#define _XOPEN_SOURCE
#include <unistd.h>
#undef _XOPEN_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <libgen.h>


#define Usage() fprintf(stderr, "Usage: %s <2_digits_key> <password>\n", \
                 basename(argv[0]))

int main (int argc, char *argv[]) {

  if (argc != 3) {
    Usage();
    exit(1);
  }

  if (strlen(argv[1]) != 2) {
    Usage();
    exit(1);
  }

  printf ("Salt: %s and Key: %s -> %s\n",
          argv[1], argv[2],
          crypt(argv[2], argv[1]));

  exit(0);
}
 

