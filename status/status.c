/* Check if target file: $n is up to date comparing to source files:       */
/*  $1, $2 .. $n-1. This is: target file exists and is newer that sources. */
/* exit 0  -->  $n file is ok (newer than all sources)                     */
/* exit 1  -->  $n file is older than some sources or does not exist       */
/* exit 2  -->  Error: cannot read some source files                       */
/* exit 3  -->  Argument or internal error                                 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>

#define EXIT_OK             0
#define EXIT_NOK            1
#define EXIT_SRC_NOT_FOUND  2
#define EXIT_INTERNAL_ERROR 3



static char *prog_name (char *prog_path) {
  char *p = strrchr(prog_path, '/');
  if (p == NULL) p = prog_path;
  else p++;
  return (p);
}

 
int main (int argc, char *argv[]) {

    struct stat bs, br;
    int result;
    int i;
    

    /* Check arguments */
    if (argc < 3)  {
        fprintf (stderr, "SYNTAX ERROR. Usage : %s { <source_file> } <result_file>\n",
                         prog_name(argv[0]));
        exit (EXIT_INTERNAL_ERROR);
    }

    /* Check that target file exists and get its modif date */
    result = EXIT_OK;
    if (stat (argv[argc-1], &br) != 0) {
        if (errno == ENOENT) {
            /* Result file not found */
            result = EXIT_NOK;
        } else {
            perror ("stat");
            fprintf (stderr, "Cannot read status of target file %s\n", argv[argc-1]);
            exit (EXIT_INTERNAL_ERROR);
        }
    }


    /* Check that each source exists and is before result */
    for (i = 1; i <= argc-2; i++) {
        if (strcmp(argv[i], argv[argc-1]) == 0) {
          fprintf (stderr, "SEMANTIC ERROR: Source file %s is also the result file.\n",
                           argv[i]);
          fprintf (stderr, " Usage : %s { <source_file> } <result_file>\n",
                           prog_name(argv[0]));
        }
        if (stat (argv[i], &bs) != 0) {
            perror ("stat");
            fprintf (stderr, "Cannot read status of source file %s\n", argv[i]);
            result = EXIT_SRC_NOT_FOUND;
        } else {
            /* time_t (buffer.st_mtime) of last modif is int */
            if ( (result == EXIT_OK) && (br.st_mtime <= bs.st_mtime) ) {
                /* Source files exist so far and this one is after result */
                result = EXIT_NOK;
            }
        }
    }

    /* Done */
    exit (result);
}

