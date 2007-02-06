#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "socket.h"


#define error(msg, arg) { \
  fprintf(stderr, "ERROR : %s %s.\n", msg, arg); \
  fprintf(stderr, "Usage : udp_send <port_spec> <dest_spec>\n"); \
  fprintf(stderr, "   <port_spec> ::= <port_name> | <port_no>\n"); \
  fprintf(stderr, "   <dest_spec> ::= lan <lan_name> | host <host_name>\n"); \
  exit (1); }

int main (int argc, char *argv[]) {
    boolean ping_lan;
    soc_port port_no;
    soc_host host_no;
    soc_token soc = init_soc;
    int res;
    char buff [500];

    /* Parse command line arguments : host_name */
    if (argc != 4) error("Syntax", "");

    /* Syntax is Ping <port> lan <lan_name> or Ping <port> host <host_name> */
    if (strcmp(argv[2], "host") == 0) {
      ping_lan = false;
    } else if (strcmp(argv[2], "lan") == 0) {
      ping_lan = true;
    } else {
      error("Syntax", argv[2]);
    }


    /* Create socket */
    res = soc_open(&soc, udp_socket);
    if (res != SOC_OK) {
        error("Socket creation", soc_error(res));
    }

    /* Set destination */
    if (soc_str2host (argv[3], &host_no) == SOC_OK) {
      if (soc_str2port (argv[1], &port_no) == SOC_OK) {
        res = soc_set_dest_host_port(soc, &host_no, port_no);
      } else {
        res = soc_set_dest_host_service (soc, &host_no, argv[1]);
      }
    } else {
      if (soc_str2port (argv[1], &port_no) == SOC_OK) {
        res = soc_set_dest_name_port(soc, argv[3], ping_lan, port_no);
      } else {
        res = soc_set_dest_name_service(soc, argv[3], ping_lan, argv[1]);
      }
    }

    if (res != SOC_OK) {
        error("Setting destination", soc_error(res));
    }

    /* Send */
    strcpy (buff, "Ah que coucou!");
    res = soc_send (soc, buff, (soc_length)strlen(buff));
    if (res != SOC_OK) {
        error("Sending message", soc_error(res));
    }
    exit (0);
}

