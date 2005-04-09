#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>


#include "socket.h"
#define SIZE 8*1024
char buff[10000000];

static void error (const char *call, int res) {
    perror (call);
    fprintf (stderr, "Error %d\n", res);
    exit (1);
}

int main (int argc, char *argv[]) {
  soc_token socket = NULL;
  int res;
  int size = 0;

  if (argc > 1) {
    size = atoi(argv[1]);
  }
  if (size == 0) {
    size = SIZE;
  }

  res = soc_open(&socket, udp_socket);
  if (res != SOC_OK) error("soc_open", res);

  res = soc_set_dest_name_service (socket, "test_ipm", TRUE, "test_udp");
  if (res != SOC_OK) error("soc_set_dest_name_service", res);

  printf ("Sending %d bytes\n", size);
  res = soc_send (socket, (soc_message)buff, (soc_length)size);
  if (res != SOC_OK) error("soc_send", res);
  printf ("Done.\n");

  return 0;
}

