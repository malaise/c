#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Usage substit <search_pattern> <change_patern> { <file> }             */
/* Replace each <search_patter> with <change_pattern> in files, in ASCII */
/* mode (file size may be changed).                                      */
/* \b -> space    \t -> tab     \n -> ret   \ij -> hexa                  */

#define SUBSTIT_VERSION  "2.2"

#define EOF_CHAR (int)0x1A

static void usage (void) {
  fprintf (stderr,
  "Substit V%s\n", SUBSTIT_VERSION);
  fprintf (stderr,
  "Usage: substit <search_pattern> <change_patern> { <file> } \n");
  fprintf (stderr,
  "  pattern can be: \\b -> space   \\t -> tab   \\n -> ret   \\ij -> hexa\n");
}

#define write_char(char) {                   \
        if (putc (char, fo) == EOF) {        \
          perror ("putc");                   \
          fprintf (stderr, "Error: writing when processing file %s\n", \
                           file_name);       \
          return (0);                        \
        }                                    \
  }

/* Write pending then skipped chars */
#define flush_pending_chars() {              \
        int iii;                             \
        for (iii = 0; iii < match; iii++) {  \
          write_char ((int)search[iii]);     \
        }                                    \
  }

static int subst_one (char *search, int ls, char *change, int lc,
                const char *file_name,
                FILE *fi, FILE *fo,
                int say_it) {
  /* Read char */
  int c;
  /* Index in search for next matching char */
  int match;
  int i;
  /* Character matches */
  int char_match;

  if (say_it) {
    printf ("processing file %s\n", file_name);
  }

  /* Search, change pattern lengths */
  ls = strlen (search);
  lc = strlen (change);

  match = 0;

  /* Infinite loop of substitutions */
  for (;;) {
    c = getc (fi);

    /* Test end of file */
    /* End of file if EOF */
    /*  flush pending chars */
    if (c == EOF) {
      flush_pending_chars ();
      /* Done */
      break;
    }

    if (c != (int) (unsigned char) search[match]) {
      /* This character does not match */
      /* Write any pending (previous matching) bytes */
      flush_pending_chars ();
      if (match == 0) {
        write_char (c);
        char_match = 0;
      } else {
        /* Reset match */
        match = 0;
        /* Check if matches first char of search */
        if (c != (int) (unsigned char) search[match]) {
          write_char (c);
          char_match = 0;
        } else {
          char_match = 1;
        }
      }
    } else {
      char_match = 1;
    }

    if (char_match) {
      /* This read character matches */
      if (match != (ls - 1) ) {
        /* Check not completed, go on (character is pending) */
        match++;
      } else {
        /* Check of pattern completed */
        /* write change */
        for (i = 0; i < lc; i++) {
          write_char (change[i]);
        }
        match = 0;
      }
    }

  } /* for */

  return (1);
}

static int ishex (char c) {
    return ( ( (c >= '0') && (c <= '9') )
          || ( (c >= 'a') && (c <= 'f') ) );
}

static unsigned char val (char c) {

    if ( (c >= '0') && (c <= '9') ) {
        return ( (unsigned char) (c - '0') );
    } else {
        return ( (unsigned char) (c - 'a') + 0x0A);
    }
}

/* \b -> space    \t -> tab     \n -> ret   \ij -> hexa */
static void format_arg (char *arg, char *format, int *p_len) {
  int i, j;

  i = 0;
  j = 0;
  for (;;) {
    if (arg[i] == '\0') {
        format[j] = '\0';
        break;
    }
    if (arg[i] != '\\') {
        format[j] = arg[i];
    } else {
        i++;
        if (arg[i] == '\0') {
            format[j] = '\\';
            j++;
            format[j] = '\0';
            break;
        }
        if (arg[i] == 'b') {
            format[j] = ' ';
        } else if (arg[i] == 't') {
            format[j] = '\t';
        } else if (arg[i] == 'n') {
            format[j] = '\n';
        } else if (ishex(arg[i]) ) {
            i++;
            if (arg[i] == '\0') {
                format[j] = '\\';
                j++;
                format[j] = arg[i-1];
                j++;
                format[j] = '\0';
                break;
            }
            if (ishex(arg[i]) ) {
                format[j] = 0x10 * val(arg[i-1]) + val(arg[i]);
            } else {
                format[j] = '\\';
                j++;
                format[j] = arg[i-1];
                j++;
                format[j] = arg[i];
            }
        } else {
            format[j] = '\\';
            j++;
            format[j] = arg[i];
            }
        }
        i++;
        j++;
    }
    *p_len = j;
}


        



int main (int argc, char *argv[]) {

  FILE *fi, *fo;
  int error = 0;
  char tmp_file_name[1024];
  int i;
  int subst_ok;
  int ls, lc;

  char to_search[1024], to_subst[1024];

  /* Check "-h" or at least 2 argument */
  if ( (argc == 2) && (strcmp(argv[1], "-h") == 0) ) {
    usage ();
    exit (1);
  }
  if (argc < 3) {
    fprintf (stderr, "Error: syntax\n");
    usage ();
    exit (1);
  }

  format_arg (argv[1], to_search, &ls);
  format_arg (argv[2], to_subst, &lc);

  for (i = 3; i < argc; i++) {

    /* Check that file exists and is read-write */
    fi = fopen (argv[i], "r+b");
    if (fi == (FILE*) NULL) {
      perror ("fopen");
      fprintf (stderr, "Error: opening file %s in read-write\n", argv[i]);
      error = 1;
      continue;
    }

    /* Build tmp_file_name */
    if (strlen(argv[i]) >= sizeof(tmp_file_name) - 5) {
      fprintf (stderr, "Error: file name %s too long\n", argv[i]);
      error = 1;
      continue;
    }
    strcpy (tmp_file_name, argv[i]);

#ifdef DOS
    {
      int j;
      int dot_found;

      /* Search last '.' (stop at last '\') */
      dot_found = 0;
      for (j = strlen(tmp_file_name) - 1; j >= 0; j--) {
        if (tmp_file_name[j] == '.') {
          dot_found = 1;
          break;
        } else if (tmp_file_name[j] == '\\') {
          break;
        }
      }
  
      /* Subst / add ".$$$" */
      if (dot_found) {
        tmp_file_name[j] = '\0';
      }
    }
#endif
    strcat (tmp_file_name, ".$$$");

    /* Create output file */
    fo = fopen (tmp_file_name, "wb");
    if (fo == (FILE*) NULL) {
      perror ("fopen");
      fprintf (stderr, "Error: opening file %s in read-write\n",
                       tmp_file_name);
      error = 1;
      continue;
    }

    /* call substitution */
    subst_ok = subst_one (to_search, ls, to_subst, lc, argv[i], fi, fo, 1);
    if (!subst_ok) error = 1;

    /* close in file */
    if (fclose(fi) == EOF) {
      perror ("fclose");
      fprintf (stderr, "Error: closing file %s\n", argv[i]);
      error = 1;
    }

    /* close out file */
    if (fclose(fo) == EOF) {
      perror ("fclose");
      fprintf (stderr, "Error: closing file %s\n", tmp_file_name);
      error = 1;
      continue;
    }

    if (subst_ok) {
      if (unlink(argv[i]) == -1) {
        perror ("unlink");
        fprintf (stderr, "Error: removing file %s\n", argv[i]);
        error = 1;
        continue;
      }

      if (rename(tmp_file_name, argv[i]) == -1) {
        perror ("rename");
        fprintf (stderr, "Error: renaming file %s in %s\n",
                         tmp_file_name, argv[i]);
        error = 1;
        continue;
      }
    } else {
      (void) unlink (tmp_file_name);
    }

  }

  if (argc == 3) {
    /* call substitution  on stdin / stdout */
    subst_ok = subst_one (to_search, ls, to_subst, lc, "stdin/out", stdin, stdout, 0);
    if (!subst_ok) error = 1;
  }

  if (error) exit (2);
  exit (0);
}


