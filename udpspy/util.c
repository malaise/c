#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <ctype.h>
#include <errno.h>


#include "boolean.h"
#include "socket.h"
#include "util.h"

/* String terminator */
#define NUL '\0'

/* Current version */
#define VERSION "0.0"

/* Exit codes */
#define ARG_ERROR_EXIT_CODE 1

/* Our program name */
static char prog_name[1024];


/* Debug on or off */
#define DEBUG_VAR "UDPSPY_DEBUG"
static boolean debug = FALSE;
static void trace (const char *msg, const char *arg) {
  if (debug) {
    fprintf (stderr, "%s->%s %s\n", prog_name, msg, arg);
  }
}

/* Result of argument parsing */
static boolean full;
static char lan_name[1024];
static soc_host lan_num;
static char port_name[1024];
static soc_port port_num;

/* Display usage message on stderr */
extern void usage (void) {
  fprintf (stderr, "Usage: %s [ -f ] <lan>:<port>\n", prog_name);
  fprintf (stderr, " <lan>  ::=  <lan_name>  | <lan_num>\n");
  fprintf (stderr, " <port> ::=  <port_name> | <port_num>\n");
  fprintf (stderr, " -f for full data dump\n");
}

/* Display an error message on stderr and exit */
extern void error (const char *msg, const char *arg) {
  fprintf (stderr, "ERROR: %s %s\n", msg, arg);
  usage();
  exit (ARG_ERROR_EXIT_CODE);
}

/* Locate a char in a string. Return relative index (0 to N) or -1 */
#define NOT_FOUND (-1)
static int locate (const char c, const char *str) {
  int i;

  /* Look for c, exit on NUL */
  i = 0;
  for (;;) {
    if (str[i] == c) {
      return i;
    } else if (str[i] == NUL) {
      break;
    }
    i++;
  }
  return NOT_FOUND;
}

/* Parse a byte from a string */
static boolean parse_byte (const char *str, const int start, const int stop, byte *b) {
  char buffer[1024];
  unsigned long l;

  /* Copy string from start to stop and NUL terminate */
  (void) strncpy (buffer, &str[start], stop - start + 1);
  buffer[stop - start + 1] = NUL;

  /* Convert to int then to byte */
  errno = 0;
  l = strtoul (buffer, NULL, 0);
  if ( (errno != 0) || (l > 255) ) {
    return FALSE;
  }
  *b = (byte) l;
  return TRUE;
}
  

/* Parse lan name or num */
static void parse_lan (const char *lan) {
  int i;
  /* Number and indexes of dots */
  int di, ds[3];
  boolean ok;
  
  trace ("Parsing lan", lan);
  /* Try to parse a LAN address byte.byte.byte.byte */
  /*  otherwise consider it is a lan name */

  /* Look for a dot, count dosts and store indexes */
  /* Check it is a digit otherwise */
  /* di must be set 3 if OK */
  di = 0;
  i = 0;
  for (;;) {
    if (lan[i] == '.') {
      /* Check and update dot number */
      if (di == 3) {
        /* Too many dots */
        di = 0;
        break;
      }
      /* Store this dot index */
      ds[di] = i;
      di++;
    } else if (lan[i] == NUL) {
      /* Done */
      break;
    } else if (! isdigit(lan[i])) {
      /* Not a dot nor a digit */
      di = 0;
      break;
    }
    i++;
  }

  /* Check Nb of dots is 3 */
  ok = (di == 3);

  /* Store bytes */
  if (ok) {
    ok  = parse_byte (lan, 0, ds[0]-1, &lan_num.bytes[0]);
  }
  if (ok) {
    ok  = parse_byte (lan, ds[0]+1, ds[1]-1, &lan_num.bytes[1]);
  }
  if (ok) {
    ok  = parse_byte (lan, ds[1]+1, ds[2]-1, &lan_num.bytes[2]);
  }
  if (ok) {
    ok  = parse_byte (lan, ds[2]+1, strlen(lan)-1,
                      &lan_num.bytes[3]);
  }

  if (ok) {
    char buffer[256];
     sprintf (buffer, "%d.%d.%d.%d", (int)lan_num.bytes[0],
              (int)lan_num.bytes[1], (int)lan_num.bytes[2],
              (int)lan_num.bytes[3]);
    trace ("Lan parsed as address", buffer);
  } else {
    /* Byte parsing failed */
    trace ("Lan parsed as name", lan);
    strcpy (lan_name, lan);
  }
  
}
 
/* Parse port name or num */
static void parse_port (const char *port) {
  int i;
  unsigned long l;
  char buffer[16];

  trace ("Parsing port", port);
  /* Check if it is digits */
  i = 0;
  for (;;) {
    if (port[i] == NUL) {
      /* Done */
      break;
    } else if (! isdigit(port[i])) {
      /* Not a digit */
      trace ("Port parsed as name", port);
      strcpy (port_name, port);
      return;
    }
    i++;
  }

  /* Convert to int then to short */
  errno = 0;
  l = strtoul (port, NULL, 0);
  if ( (errno != 0) || (l > 65535) ) {
    trace ("Port parsed as name", port);
    strcpy (port_name, port);
    return;
  }
  port_num = (soc_port) l;
  sprintf (buffer, "%d", (int) port_num);
  trace ("Port parsed as num", buffer);
}

/* Parse lan:port, exit on error */
static void parse_arg (const char *arg) {
  int column_i;
  char locarg [1024];

  /* Init to default */
  lan_name[0] = NUL;
  lan_num.integer = 0;
  port_name[0] = NUL;
  port_num = 0;

  /* Locate column in arg */
  column_i = locate (':', arg);
  if ( (column_i == NOT_FOUND) 
    || (column_i == 1) 
    || (column_i == (int)strlen(arg)-1) ) {
    error ("invalid argument", arg);
  }

  /* Save arg, split and parse lan and port */
  strcpy (locarg, arg);
  locarg[column_i] = NUL;
  parse_lan (&locarg[0]);
  parse_port (&locarg[column_i+1]);

}

/* Parse arguments, exit on error */
extern void parse_args (const int argc, const char *argv[]) {
  char buffer [sizeof(prog_name)];
  char *debugstr;

  /* Store program name (basename alters buffer) */
  strcpy (buffer, argv[0]);
  strcpy (prog_name, basename(buffer));

  /* Set debug */
  debugstr = getenv (DEBUG_VAR);
  if ( (debugstr != NULL) && (toupper(debugstr[0]) == 'Y') ) {
   debug = TRUE;
  }

  /* Parse arguments */
  if (argc == 1) {
    error ("missing argument", "");
  } else if (argc == 2) {
    if ( (strcmp (argv[1], "-h") == 0)
      || (strcmp (argv[1], "--help") == 0) ) {
      usage();
      exit (ARG_ERROR_EXIT_CODE);
    } else if ( (strcmp (argv[1], "-v") == 0)
             || (strcmp (argv[1], "--version") == 0) ) {
      fprintf (stderr, "%s version %s\n", prog_name, VERSION);
      exit (ARG_ERROR_EXIT_CODE);
    } else {
      full = FALSE;
      parse_arg (argv[1]);
    }  
  } else if (argc == 3) {
    if (strcmp (argv[1], "-f") == 0) {
      trace ("Full detected as arg 1", "");
      full = TRUE;
      parse_arg (argv[2]);
    } else if (strcmp (argv[2], "-f") == 0) {
      trace ("Full detected as arg 2", "");
      full = TRUE;
      parse_arg (argv[1]);
    } else {
      trace ("Full not detected", "");
      error ("invalid arguments", "");
    }
  } else {
    error ("too many arguments", "");
  }
}

