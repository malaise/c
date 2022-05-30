#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "boolean.h"

#define SUFFIX ".tmp"

#define write_char(c) {if (putc((int)c, fo) == EOF) { \
                         perror ("putc"); \
                         fprintf (stderr, "Error writting on %s\n", file_name); \
                         break; \
                        } \
                      }

static void convert (char *file_name) {
  char *unix_file_name;
  FILE *fi, *fo;
  int c;
  char c1;

  unix_file_name = malloc(strlen(file_name) + sizeof(SUFFIX));
  if (unix_file_name == (char*) NULL) {
    perror ("malloc");
    fprintf (stderr, "Error allocating file name %s%s\n", file_name, SUFFIX);
    return;
  }
  strcpy (unix_file_name, file_name);
  strcat (unix_file_name, ".tmp");


  if ( (fi = fopen (file_name, "rb")) == (FILE*)NULL) {
    perror ("fopen");
    fprintf (stderr, "Error openning %s for reading\n", file_name);
    free (unix_file_name);
    return;
  }

  if ( (fo = fopen (unix_file_name, "wb")) == (FILE*)NULL) {
    perror ("fopen");
    fprintf (stderr, "Error openning %s for writting\n", unix_file_name);
    (void) fclose(fi);
    free (unix_file_name);
    return;
  }

  /* 0D 0A --> 0A */
  /* 1A    --> Close */
  for (;;) {
    c = getc(fi);
    if (c == (char) 0x0D) {
      c1 = getc(fi);
      if (c1 != (char) 0x0A) {
        write_char(c);
      }
      write_char(c1);
    } else if ( (c == 0x1A) || (c == EOF) ) {
      break;
    } else {
      write_char(c);
    }
  }

  if (fclose(fi) != 0) {
    perror ("fclose");
    fprintf (stderr, "Error closing %s after processing\n", file_name);
  }

  if (fclose(fo) != 0) {
    perror ("fclose");
    fprintf (stderr, "Error closing %s after processing\n", unix_file_name);
  }

  if (unlink (file_name) == -1) {
    perror ("unlink");
    fprintf (stderr, "Error removing %s after processing\n",
                     file_name);
    free (unix_file_name);
    return;
  }
  if (rename (unix_file_name, file_name) == -1) {
    perror ("rename");
    fprintf (stderr, "Error renaming %s to %s after processing\n",
                      unix_file_name, file_name);
    free (unix_file_name);
    exit (1);
  }
  free (unix_file_name);

}

int main (int argc, char *argv[]) {


  int cur_arg;

  for (cur_arg=2; cur_arg <= argc; cur_arg++) {
    printf ("Converting dos to unix : %s\n", argv[cur_arg-1]);
    convert(argv[cur_arg-1]);
  }
  printf ("Done.\n");
  exit (0);
}

