#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "boolean.h"
#include "socket.h"

static void usage (void) {
  fprintf (stderr, "ERROR. Only one argument, host name or ip address, accepted\n");
}

int main (int argc, char *argv[]) {

  soc_host host;
  boolean ip_addr;
  int ndots;
  int i;
  char *p;
  char addr[500];
  int res;

  if (argc != 2) {
    usage();
    exit(2);
  }

  ip_addr = TRUE; 
  ndots = 0;

  for (i = 0; i < strlen(argv[1]); i++) {
    if (argv[1][i] == '.') {
      ndots ++;
    } else if (! isdigit(argv[1][i])) {
      ip_addr = FALSE;
    }
  }
  
  if (ip_addr && (ndots != 3)) {
    usage();
  }


  if (ip_addr) {
    strcpy (addr, argv[1]);
    p = addr;
    ndots = 0;
    for (i = 0; i < strlen(argv[1]); i++) {
      if (addr[i] == '.') {
        addr[i] = '\0';
        host.bytes[ndots] = atoi(p);
        p = &(addr[i+1]);
        ndots++;
      }
    }
    host.bytes[ndots] = atoi(p);

    printf("Looking for ip addr: ");
    for (i = 0; i <= 3; i++) {
      printf("%d", (int)host.bytes[i]);
      if (i != 3) {
        printf(".");
      }
    }
    printf("\n");
    res = soc_host_name_of (&host, addr, sizeof(addr));
    if (res != SOC_OK) {
      fprintf (stderr, "soc_host_name_of -> %d\n", res);
      exit (1);
    } else {
      printf ("Got: %s\n", addr);
    }
  } else {
    printf("Looking for host name: %s\n", argv[1]);
    res = soc_host_of (argv[1], &host);
    if (res != SOC_OK) {
      fprintf (stderr, "soc_host_of -> %d\n", res);
      exit (1);
    } else {
      printf("Got: ");
      for (i = 0; i <= 3; i++) {
        printf("%d", (int)host.bytes[i]);
        if (i != 3) {
          printf(".");
        }
      }
      printf("\n");
    }

  }
  exit (0);
}


