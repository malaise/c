#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <unistd.h>
#include <errno.h>

/* Read stdin (a pipe) flow and outputs it in a circular file (argv[1]) */
/*  of a given size (arv[2]) */

#include "circul.h"

#define INTERNAL_BUFFER_SIZE  1024
char buffer[INTERNAL_BUFFER_SIZE];


int main (int argc, char *argv[]) {

  unsigned int size;
  int nb_read;
  struct cir_file *circul_fd = NULL;

  if (argc < 2) {
    fprintf (stderr, "Syntax error. Syntax %s <circular_file_name>"
                     " <circular_file_size>\n", argv[0]);
    exit (1);
  }

  size = strtol (argv[2], NULL, 10);
  if ( (size == 0) || (errno != 0) ) {
    fprintf (stderr, "Invalid size %s\n", argv[2]);
    exit (1);
  }
  
  /* Open output circular file */
  circul_fd = cir_open(argv[1], "w+", size);
  if (circul_fd == NULL) {
    fprintf(stderr, "Cannot open the circular file %s\n", argv[1]);
    exit (1);
  }

  /* Read stdin and write circ */
  for (;;) {
    nb_read = read (0, buffer, sizeof(buffer));
    if (nb_read > 0) {
      if (cir_write (circul_fd, buffer, nb_read) == -1 ) {
        fprintf(stderr, "Failed to write in the circular file\n");
        (void) cir_close (circul_fd);
        exit (1);
      }
    } else if (nb_read < 0) {
      perror ("read");
      exit (1);
    } else {
      /* End of input */
      break;
    }
  }

  /* Close */
  if (cir_close (circul_fd) == -1) {
    fprintf(stderr, "Failed to close the circular file\n");
    exit (1);
  }

  /* Done */
  exit (0);

}


