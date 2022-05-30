#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "get_line.h"

/* Remove all <....> from a file (argv[1]). Save in <file>-ascii */
/* Or read stdin and write on stdout */
/* Skip all leading spaces */
/* Replace each sequence if 5 spaces within line by 1 space */

#define BUFFER_SIZE 1024

static void error (const char *prog, const char *msg)
                  __attribute__((noreturn));
static void error (const char *prog, const char *msg) {
  fprintf (stderr, "ERROR: %s.\n", msg);
  fprintf (stderr, "Usage: %s [ <html_file> ]\n", prog);
  exit(1);
}

const char sstart[] = "Vos Documents au format PDF:";
const char sstop[] = "&nbsp;";


int main (int argc, char *argv[]) {

   FILE *fin, *fout;
   char out_file_name[1024];
   char buffer[BUFFER_SIZE];
   int len;
   int i, j;
   int skip, sskip;
   int nb_space;
   int leading;
   int nb_empty;

   if (argc > 2) {
     error (argv[0], "invalid arguments"); 
   }

   if (argc == 1) {
     fin = stdin;
     fout = stdout;
   } else {
     fin = fopen (argv[1], "r");
     if (fin == (FILE*) NULL) {
       error (argv[0], "can't open input file");
     }
     strcpy (out_file_name, argv[1]);
     strcat (out_file_name, "-ascii");
     fout = fopen (out_file_name, "w");
     if (fout == (FILE*) NULL) {
       fclose (fin);
       error (argv[0], "can't create output file");
     }
  }

  skip = 0;
  sskip = 0;
  nb_empty = 0;
  for (;;) {

    nb_space = 0;
    leading = 1;
    len = get_line (fin, buffer, BUFFER_SIZE);
    if (len == -1) {
      break;
    }
    j = 0;
    for (i = 0; i <= len; i++) {
      if (buffer[i] == '<') {
        skip = 1;
      } else if (buffer[i] == '>') {
        skip = 0;
      } else {

        if (!skip) {
          /* We're out of < .. > */
          /* Skip all leading spaces */
          if (buffer[i] == ' ') {
            if (! leading) {
              /* Within line, 5 spaces become 1 space */
              nb_space ++;
              if (nb_space == 5) {
                j -= 4;
                nb_space = 0;
              } 
            }
          } else {
            /* One significant character: within line, no more space */
            leading = 0;
            nb_space = 0;
          }
          if (!leading) {
            if (buffer[i] == (char)0xE9) {
              buffer[j] = 'e';
              j++;
            } else {
              buffer[j] = buffer[i];
              j++;
            }
          }
        }

      }
    }
    /* Now check super start and super stop */
    if ( (sskip == 0) && (strncmp(buffer, sstart, strlen(sstart)) == 0) ) {
      sskip = 1;
    }
    if ( (sskip == 1) && (strncmp(buffer, sstop, strlen(sstop)) == 0) ) {
      sskip = 2;
    }
    if (sskip == 1) {
      /* Only one empty line */
      if (strlen(buffer) == 0) {
        nb_empty ++;
      } else {
        nb_empty = 0;
      }
      if (nb_empty <= 1) {
        fprintf (fout, "%s\n", buffer);
      }
    }
  }

     
  if (argc != 1) {
   fclose (fin);
   fclose (fout);
  }
  exit(0);
}

