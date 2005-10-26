#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <unistd.h>
#include <errno.h>

#include "circul.h"


#define INTERNAL_BUFFER_SIZE  512
char buffer[INTERNAL_BUFFER_SIZE];

#define output_file_suffix ".output.tmp"
char output_file_name[MAXHOSTNAMELEN + sizeof(output_file_suffix) + 1];
int output_fd;


static int uncircular_one (char *input_file_name) {

int char_read;
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
    if (char_read == -1) {
      fprintf(stderr, "Cannot read on circular file %s\n", input_file_name);
      return (1);
    }

    if (write(output_fd, buffer, char_read) != char_read) {
      fprintf(stderr, "Cannot write (%d bytes) on linear file %s\n",
              char_read, output_file_name);
      return (1);
    }
  } while ( (char_read != 0) && (char_read != -1));

  strcpy (buffer, "End of circular file\n");
  char_read = strlen(buffer);
  if (write(output_fd, buffer, char_read) != char_read) {
    fprintf(stderr, "Cannot write \"End of circular file\" on linear file %s", output_file_name);
    return (1);
  }
  cir_close (circul_fd);
  circul_fd = NULL;
  return (0);
}

int main (int argc, char *argv[]) {
int result;
int i;


  if (argc < 2) {
    fprintf (stderr, "Syntax error. Syntax %s { <circular_file_name> }\n", argv[0]);
    exit (1);
  }

  /* Build output file name: <hostname>.output.tmp */
  (void) gethostname (output_file_name, sizeof(output_file_name));
  strcat (output_file_name, output_file_suffix);

  /* Open output file */
  do {
    output_fd = open(output_file_name, O_WRONLY|O_CREAT|O_TRUNC, 0777);
  } while ( (output_fd == -1) && (errno == EINTR));
  if (output_fd == -1) {
    fprintf(stderr, "Cannot create the linear file %s (errno = %d)", output_file_name, errno);
    exit (1);
  }

  /* Loop on each argument */
  result = 0;
  for (i = 1; i < argc; i++) {
      result += uncircular_one(argv[i]);
  }

  strcpy (buffer, "\n");
  if (write(output_fd, buffer, 1) != 1) {
    fprintf(stderr, "Cannot write last \\n on linear file %s", output_file_name);
    exit (1);
  }
  close (output_fd);

  exit (result != 0);
}


