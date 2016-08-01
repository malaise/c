#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <unistd.h>
#include <errno.h>

/* Uncircular and concat several circular files into stdout */

#include "circul.h"

#define INTERNAL_BUFFER_SIZE  512
char buffer[INTERNAL_BUFFER_SIZE];

static int uncircular_one (char *input_file_name) {

size_t char_read;
struct cir_file *circul_fd = NULL;

  /* Generate linear file */
  circul_fd = cir_open(input_file_name, "r", 0);
  if (circul_fd == NULL) {
    fprintf(stderr, "Cannot open the circular file %s\n", input_file_name);
    return (1);
  }

  do {
    char_read = 0;
    char_read = cir_read(circul_fd, buffer, INTERNAL_BUFFER_SIZE);
    if ((int) char_read == -1) {
      fprintf(stderr, "Cannot read on circular file %s\n", input_file_name);
      return (1);
    }

    if (fwrite(buffer, 1, char_read, stdout) != char_read) {
      fprintf(stderr, "Cannot write (%ld bytes) on stdout\n", char_read);
      return (1);
    }
  } while ( (char_read != 0) && ((int) char_read != -1));

  cir_close (circul_fd);
  circul_fd = NULL;
  return (0);
}

int main (int argc, char *argv[]) {
int result;
int i;


  if (argc < 2) {
    fprintf (stderr, "Syntax error. Syntax %s { <circular_file_name> }\n",
             argv[0]);
    exit (1);
  }

  /* Process each argument */
  result = 0;
  for (i = 1; i < argc; i++) {
      result += uncircular_one(argv[i]);
  }

  if (fwrite("\n", 1, 1, stdout) != 1) {
    fprintf(stderr, "Cannot write last \\n on stdout\n");
    exit (1);
  }

  exit (result != 0);
}


