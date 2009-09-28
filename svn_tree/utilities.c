  /****************************************/
 /*** Utility functions for "svn_tree" ***/
/****************************************/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <libgen.h>
#include <errno.h>

#include "utilities.h"

/* Level set by getenv */
static const char *DEBUG_NAME="SVN_TREE_DEBUG";
static int debug_level = -1;

/* In any kind of fatal error */
extern void error_data (const char *msg, const char *data)
  __attribute__ ((noreturn));
extern void error_data (const char *msg, const char *data) {
  fprintf (stderr, "ERROR: %s %s\n", msg, data);
  exit (1);
}

/* In any kind of fatal error */
extern void error (const char *msg)
  __attribute__ ((noreturn));
extern void error (const char *msg) {
  fprintf (stderr, "ERROR: %s\n", msg);
  exit (1);
}

/* Put command line help */
extern void put_help (const char *prog) {
  url_t local_prog;
  strcpy (local_prog, prog);
  fprintf(stderr,"%s <what> [ <where> ]\n", basename (local_prog));
  fprintf(stderr, " <what>  ::= <url> | CUR | ALLTAGS\n");
  fprintf(stderr, " <where> ::= -of | -on  <onwhat>\n");
  fprintf(stderr, " <onwhat> ::= <url> | CUR\n\n");
  fprintf(stderr, "Print info (source and dest) of the provided <url>\n");
  fprintf(stderr, "  or of current sandbox or of all tags.\n");
  fprintf(stderr, "  <url> or CUR must be a label (branch or tag)\n");
  fprintf(stderr, "Option <where> prints info only if label is on tree\n");
  fprintf(stderr, "  (either provided <url> or current sandbox),\n");
  fprintf(stderr, "  including (-of) or excluding (-on) its ancestors.\n");
}

/***********************************/
/* Functions to deal with strings */
/*********************************/

/* Check whether string begins with URL start symbols "://" */
extern boolean  is_url(const char *url) {
  const char *url_start = "://";
  char *p;

  p = strstr(url, url_start);
  if ( (p == NULL) || (p == url) ) {
    /* Invalid or empty scheme */
    return FALSE;
  }
  if (*(p + 3) == '\0') {
    /* Empty URL after scheme */
    return FALSE;
  }
  if ( (*(p + 3) == '/')
    && (*(p + 4) == '\0') ) {
    /* Empty URL after scheme */
    return FALSE;
  }
  return TRUE;
}

static const char *branch = "/branches/";
static const char *tag = "/tags/";
static const char *trunk = "/trunk";

/* Check program input argument URL */
extern char *url_check (const char *url, boolean allow_trunk) {
  char *p, *ps[3];
  int i;

  if (strlen(url) > URL_MAX_LEN) {
    debug2 (1, "url_check with too long url", url);
    error("URL is too long");
  }
  /* Try to locate any label, store smallest non null */
  ps[0] = strstr(url, branch);
  ps[1] = strstr(url, tag);
  ps[2] = strstr(url, trunk);
  p = NULL;
  for (i = 0; i <= 2; i++) {
    if ( (p == NULL) || (p < ps[i]) ) {
      p = ps[i];
    }
  }
  if (p == NULL) {
    return p;
  }

  if ( !allow_trunk && (strstr(p, trunk) == p) ) {
    /* This URL is on trunk, which is not allowed */
    return NULL;
  }

  return p;
}

/* Get short label (trunk, branches/name or tags/name) */
extern void get_node_path (char *string) {
  int nb;
  int count= 0;
  int i;
  label_kind lab;

  /* Nb of / to pass before cutting */
  lab = compare_path (string);
  switch (lab) {
    case TRUNK:
      nb = 2;
    break;
    case TAG:
    case BRANCH:
      nb = 3;
    break;
    default:
      return;
  }

  /* Cut at nb '/'s */
  for (i = 0; i < (int) strlen (string); i++) {
    if (string[i] == '/') {
      count++;
      if (count == nb) {
        string[i] = '\0';
      }
    }
  }
}

/* Check that the action is not against a file.
   Function checks number of '/' in key path. */
extern label_kind compare_path (const char *path) {
  int count = 0;
  int i;
  char *p;

  /* Scanning string for end char. If '/' appears  more than twice,
     then this is a copy against a file -> INVALID */
  for (i = 0; path[i] != '\0'; i++) {
    if (path[i] == '/') {
      count++;
    }
  }
  if (count > 2) {
    return INVALID;
  }
  p = url_check (path, TRUE);
  if (p == NULL) {
    return INVALID;
  }

  if (strstr(path, branch) == p) {
    return BRANCH;
  }
  if (strstr(path, tag) == p) {
    return TAG;
  }
  if (strstr(path, trunk) == p) {
    /* Only "/trunk" accepted */
    if (count > 1) {
      return INVALID;
    }
    return TRUNK;
  }
  return INVALID;
}

/* Modifying SVN output date to readable format */
extern void svn_date_to_normal (char *date) {
  int i = 0;
  for (i = 0; date[i] != '\0'; i++) {
     if (date[i] == '.') {
         date[i] = '\0';
     }
  }
}

/* Output debug info */
static boolean putit (int severity) {
  char *p;
  if (debug_level == -1) {
    /* Done at first call */
    p = getenv (DEBUG_NAME);
    if (p == NULL) {
      /* ENV var not set */
      debug_level = 0;
    } else {
      /* Convert to int */
      errno = 0;
      debug_level = (int) strtol (p, NULL, 0);
      if (errno != 0) {
        /* Env var is not int => set to 1 */
        debug_level = 1;
      }
    }
  }
  return (severity <= debug_level);
}

extern void debug1 (int severity, const char *msg) {
  if (! putit (severity) ) return;
  fprintf (stderr, "-> %s\n", msg);
}

extern void debug2 (int severity, const char *msg, const char *data) {
  if (! putit (severity) ) return;
  fprintf (stderr, "-> %s, %s\n", msg, data);
}

extern void debug3 (int severity, const char *msg, const char *data,
                                  const char *extra) {
  if (! putit (severity) ) return;
  fprintf (stderr, "-> %s, %s, %s\n", msg, data, extra);
}

