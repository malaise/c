#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <ctype.h>
#include <errno.h>


#include "boolean.h"
#include "socket.h"
#include "timeval.h"
#include "util.h"

/* String terminator */
#define NUL '\0'

/* Current version */
#define VERSION "1.0"

/* Exit code */
#define ERROR_EXIT_CODE 1

/* Our program name */
static char prog_name[1024];


/* Debug on or off */
#define DEBUG_VAR "UDPSPY_DEBUG"
static boolean debug = FALSE;
extern void trace (const char *msg, const char *arg) {
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
  exit (ERROR_EXIT_CODE);
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

/* Parse lan name or num */
static void parse_lan (const char *lan) {
  
  trace ("Parsing lan", lan);
  /* Try to parse a LAN address byte.byte.byte.byte */
  /*  otherwise consider it is a lan name */

  if (soc_str2host (lan, &lan_num) == SOC_OK) {
    char buffer[16];
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

  /* Trye to convert to port num */
  if (soc_str2port (port, &port_num) == SOC_OK) {
    char buffer[6];
    sprintf (buffer, "%d", (int) port_num);
    trace ("Port parsed as num", buffer);
  } else {
    trace ("Port parsed as name", port);
    strcpy (port_name, port);
  }
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
  /* Only one clumn */
  if (locate (':', &arg[column_i+1]) != NOT_FOUND) {
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
      exit (ERROR_EXIT_CODE);
    } else if ( (strcmp (argv[1], "-v") == 0)
             || (strcmp (argv[1], "--version") == 0) ) {
      fprintf (stderr, "%s version %s\n", prog_name, VERSION);
      exit (ERROR_EXIT_CODE);
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

/* Bind the socket to the IPM lan and port */
extern void bind_socket (soc_token socket) {
  int res1, res2;
  char buffer[255];

  /* Set dest and bind */
  if (strlen(port_name) == 0) {
    if (strlen(lan_name) == 0) {
      /* Lan num and port num */
      res1 = soc_set_dest_host_port (socket, &lan_num, port_num);
    } else {
      /* Lan name and port num */
      res1 = soc_set_dest_name_port (socket, lan_name, TRUE, port_num);
    }
    res2 = (res1 == SOC_OK) && soc_link_port (socket, port_num);
  } else {
    if (strlen(lan_name) == 0) {
      /* Lan num and port name */
      res1 = soc_set_dest_host_service (socket, &lan_num, port_name);
    } else {
      /* Lan name and port name */
      res1 = soc_set_dest_name_service (socket, lan_name, TRUE, port_name);
    }
    res2 = (res1 == SOC_OK) && soc_link_service (socket, port_name);
  }

  if (res1 != SOC_OK) {
    sprintf (buffer, "%d", res1);
    trace ("soc_set_dest error", "buffer");
    error ("cannot set socket dest", "");
  }
  if (res2 != SOC_OK) {
    sprintf (buffer, "%d", res2);
    trace ("soc_link error", "buffer");
    error ("cannot link socket", "");
  }

}

/* Put message info */
extern void display (const char *message, const int length) {

  /* Put time */
  printf ("Received message\n");
  /* Put from */
  /* Put data if full */
  if (!full) {
    return;
  }
  printf ("%d %s\n", length, message);

}

