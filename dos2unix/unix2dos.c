#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#include "boolean.h"

#define SUFFIX ".tmp"

#define write_char(c) {if (putc((int)c, fo) == EOF) { \
                         perror ("putc"); \
                         fprintf (stderr, "Error writting on %s\n", file_name); \
                         break; \
                        } \
                      }

static void convert (char *file_name) {
  char *dos_file_name;
  FILE *fi, *fo;
  char c;

  dos_file_name = malloc(strlen(file_name) + sizeof(SUFFIX));
  if (dos_file_name == (char*) NULL) {
    perror ("malloc");
    fprintf (stderr, "Error allocating file name %s%s\n", file_name, SUFFIX);
    return;
  }
  strcpy (dos_file_name, file_name);
  strcat (dos_file_name, ".tmp");

  if ( (fi = fopen (file_name, "rb")) == (FILE*)NULL) {
    perror ("fopen");
    fprintf (stderr, "Error openning %s for reading\n", file_name);
    free (dos_file_name);
    return;
  }

  if ( (fo = fopen (dos_file_name, "wb")) == (FILE*)NULL) {
    perror ("fopen");
    fprintf (stderr, "Error openning %s for writting\n", dos_file_name);
    (void) fclose(fi);
    free (dos_file_name);
    return;
  }

  /* 0A --> 0D 0A */
  /* EOF --> 1A Close */
  for (;;) {
    c = getc(fi);
    if (c == (char) 0x0A) {
      write_char(0X0D);
      write_char(c);
    } else if (c == (char) EOF) {
      write_char(0x1A);
      break;
    } else {
      write_char(c);
    }
  }

  if (fclose(fi) != 0) {
    perror ("fclose");
    fprintf (stderr, "Error closing %s after processing\n", file_name);
    free (dos_file_name);
  }

  if (fclose(fo) != 0) {
    perror ("fclose");
    fprintf (stderr, "Error closing %s after processing\n", dos_file_name);
    free (dos_file_name);
  }

  if (unlink (file_name) == -1) {
    perror ("unlink");
    fprintf (stderr, "Error removing %s after processing\n",
                     file_name);
    free (dos_file_name);
    return;
  }
  if (rename (dos_file_name, file_name) == -1) {
    perror ("rename");
    fprintf (stderr, "Error renaming %s to %s after processing\n",
                      dos_file_name, file_name);
    free (dos_file_name);
    return;
  }

  free (dos_file_name);
}

int main (int argc, char *argv[]) {


  int cur_arg;

  for (cur_arg=2; cur_arg <= argc; cur_arg++) {
    printf ("Converting unix to dos : %s\n", argv[cur_arg-1]);
    convert(argv[cur_arg-1]);
  }
  printf ("Done.\n");
  exit (0);
}

