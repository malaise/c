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
  printf (" <==> %s\n", name);
}


int main (int argc, char *argv[]) {

  int i;
  int res, ecode;
  soc_host host;
  char host_name[MAXHOSTNAMELEN];

  ecode = 0;
  if (argc == 1) {
    if ( (res = soc_get_local_host_name (host_name, sizeof(host_name)) ) != SOC_OK) {
      fprintf (stderr, "soc_get_local_host_name -> %d\n", res);
      ecode = 1;
    }
    if (ecode == 0) {
      if ( (res = soc_get_local_host_id (&host) ) != SOC_OK) {
        fprintf (stderr, "soc_get_local_host_id -> %d\n", res);
        ecode = 1;
      }
    }
    if (ecode == 0) {
      print_ip (&host, host_name);
    }
  } else if (strcmp (argv[1], "-h") == 0) {
    printf ("Usage: %s [ { <host_name> } ]\n", basename(argv[0]));
    ecode = 2;
  } else {
    for (i = 1; i < argc; i++) {
      if ( (res = soc_host_of (argv[i], &host) ) != SOC_OK) {
        fprintf (stderr, "soc_host_of(%s) -> %d\n", argv[i], res);
        ecode = 1;
      } else {
        print_ip (&host, argv[i]);
      }
    }
  }

  exit (ecode);
}

