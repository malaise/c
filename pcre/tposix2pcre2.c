#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <posix2pcre.h>

static void usage (char *name) {
  fprintf (stderr, "Usage: %s <regex> { [ <string> ] }\n", name);
}

static void print_match (regmatch_t match) {
  printf ("Match at %d-%d\n", match.rm_so, match.rm_eo - 1);
}

#define MAX_MATCHES 100
int main (int argc, char *argv[]) {

  int i, j, n, res;
  regex_t regex;
  char buffer[1024];
  const char *pchar;
  regmatch_t matches[MAX_MATCHES];

  /* At least one argument, -h | --help | <regex> */
  if (argc < 2) {
    usage (argv[0]);
    exit (1);
  }

  /* Help */
  if ( (strcmp (argv[1], "-h") == 0) || (strcmp (argv[1], "--help") == 0) ) {
    usage (argv[0]);
    exit (0);
  }
  /* Show version */
  if ( (strcmp (argv[1], "-v") == 0) || (strcmp (argv[1], "--version") == 0) ) {
    pchar = pcre_version ();
    printf ("Pcre version is %s\n", pchar);
    exit (0);
  }

  /* Compile regex */
  res = posix2pcre_regcomp (&regex, argv[1], 0);
  if (res != 0) {
    (void) posix2pcre_regerror (res, &regex, buffer, sizeof(buffer));
    fprintf (stderr, "ERROR: Compilation has failed with error: %s\n",
             buffer);
    exit (1);
  }

  /* Check strings */
  for (i = 3; i <= argc; i++) {
    /* Compile */
    res = posix2pcre_regexec (&regex, argv[i-1], MAX_MATCHES, matches, 0);
    if (res == REG_NOMATCH) {
      printf ("No match\n");
      continue;
    }
    if (res != 0) {
      (void) posix2pcre_regerror (res, &regex, buffer, sizeof(buffer));
      fprintf (stderr, "ERROR: Execution has failed with error: %s\n",
               buffer);
      exit (1);
    }
    /* Print result */
    print_match (matches[0]);

    /* Count matching substrings */
    n = 0;
    for (j = MAX_MATCHES - 1; j >= 0; j--) {
      if (matches[j].rm_so != -1) {
        n = j;
        break;
      }
    }

    /* Print matching substrings */
    for (j = 1; j <= n; j++) {
      print_match (matches[j]);
    }
  }

  /* Cleanup */
  posix2pcre_regfree (&regex);
    
  /* Done */
  exit (0);
}

