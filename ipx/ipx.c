#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libgen.h>
#include <sys/param.h>

#include "socket.h"

static void print_ip (soc_host *host, char *name) {
  int i;

  for (i = 0; i <= 3; i++) {
    printf ("%02X", host->bytes[i]);
  }
  printf (" <==> ");
  for (i = 0; i <= 3; i++) {
    printf ("%d", host->bytes[i]);
    if (i != 3) {
      printf (".");
    }
  }
  printf (" <==> ");
  if (name[0] == '\0') {
    printf ("?");
  } else {
    printf ("%s", name);
  }
  printf ("\n");
}

static byte b2h (char c) {
  if ( (c >= '0') && (c <= '9') ) {
    return c - '0';
  } else if ( (c >= 'a') && (c <= 'f') ) {
    return 10 + c - 'a';
  } else if ( (c >= 'A') && (c <= 'F') ) {
    return 10 + c - 'A';
  } else {
    return 0;
  }
}

char host_name[MAXHOSTNAMELEN];

static void do_one (char *host_name) {
  int i;
  int res;
  char c;
  soc_host host;

  /* Try with host name or ip notation */
  if ( (res = soc_host_of (host_name, &host) ) == SOC_OK) {
    if (soc_host_name_of (&host, host_name, sizeof(host_name)) != SOC_OK) {
      /* Host not found */
      host_name[0] = '\0';
    }
    print_ip (&host, host_name);
  } else  if (strlen (host_name) == 8) {
    /* Try with 8 hexa digits */
    res = 0;
    for (i = 0; i < 8; i++) {
      c = host_name[i];
      if ( ( (c >= '0') && (c <= '9') )
        || ( (c >= 'a') && (c <= 'f') )
        || ( (c >= 'A') && (c <= 'F') ) ) {
        ;
      } else {
        res = 1;
      }
    }
    if (res == 0) {
      /* Looks like a host id */
      for (i = 0; i < 8; i++, i++) {
        host.bytes[i/2] = (b2h(host_name[i]) * 16) + b2h(host_name[i+1]);
      }
      if (soc_host_name_of (&host, host_name, sizeof(host_name)) != SOC_OK) {
        /* Host not found */
        host_name[0] = '\0';
      }
      print_ip (&host, host_name);
    }  else {
      fprintf (stderr, "Invalid argument %s\n", host_name);
    }
  }
}

int main (int argc, char *argv[]) {

  int i;
  int res;

  if (argc == 1) {
    /* No arg => local host */
    if ( (res = soc_get_local_host_name (host_name, sizeof(host_name)) ) == SOC_OK) {
      do_one (host_name);
    } else {
      fprintf (stderr, "soc_get_local_host_name -> %d\n", res);
      exit (1);
    }
  } else if (strcmp (argv[1], "-h") == 0) {
    printf ("Usage: %s [ { <host_name> | <host_ip> | <host_id> } ]\n",
            basename(argv[0]));
    exit (2);
  } else {
    /* Dump each argument */
    for (i = 1; i < argc; i++) {
      do_one (argv[i]);
    }
  }
  exit (0);
}

