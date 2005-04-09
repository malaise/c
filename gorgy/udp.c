#include <ctype.h>
#include <stdlib.h>

#include "socket.h"
#include "udp.h"

static soc_token soc = NULL;

void init_udp (char *port_name_no) {

  int port_no;
  int cr;
  char lan_name[80];

  if (soc_get_lan_name(lan_name, sizeof(lan_name)) != SOC_OK) {
    perror("getting lan name");
    exit (2);
  }

  soc = init_soc;


  if (soc_open(&soc, udp_socket) != SOC_OK) {
    perror ("opening socket");
    exit (2);
  }

  if (isdigit(port_name_no[0])) {
    port_no = 0;
    port_no = atoi(port_name_no);
    if (port_no == 0) {
      fprintf (stderr, "Wrong port number >%s<\n", port_name_no);
      exit (2);
    } else {
      cr = soc_set_dest_name_port (soc, lan_name, true, (soc_port) port_no);
    }
  } else {
    cr = soc_set_dest_name_service  (soc, lan_name, true, port_name_no);
  }
  
  if (cr != SOC_OK) {
    perror ("setting detination");
    exit (2);
  }

  if (soc_get_dest_port(soc, (soc_port*) &port_no) != SOC_OK) {
    perror ("getting port no");
    exit (2);
  }

  printf ("UDP port no %d initialised.\n", port_no);

}

void send_udp (char *msg, int len) {

  if (soc_send (soc, (soc_message) msg, (soc_length) len) != SOC_OK)
    perror ("sending");
}

