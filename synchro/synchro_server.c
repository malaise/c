#include <stdio.h>
#include <stdlib.h>

#ifdef UBSS
#include "bs.h"
#include "scm.h"
#include "npm.h"
#endif

#include "socket.h"
#include "timeval.h"

#include "synchro.h"

#define USAGE() {fprintf(stderr, "Usage : synchro_server <port_name>\n");}

int main (int argc, char *argv[]) {

  soc_token soc = init_soc;
  soc_port port_no;
  synchro_msg_t synchro_msg;
  soc_length length;
  int cr;

#ifdef UBSS
  if (npm_bs_init()==BS_ERROR) {
    scm_bscall_error();
    exit (1);
  }
#endif

  if (argc != 2) {
    fprintf (stderr, "SYNTAX error : Wrong number of arguments\n");
    USAGE();
    exit (1);
  }

  /* Create socket and bind to service */
  if (soc_open(&soc, udp_socket) != SOC_OK) {
    perror ("opening socket");
    exit (2);
  }

  port_no = atoi(argv[1]);
  if (port_no <= 0) {
    if (soc_link_service(soc, argv[1]) != SOC_OK) {
      perror ("linking socket");
      exit (1);
    }
  } else {
    if (soc_link_port(soc, port_no) != SOC_OK) {
      perror ("linking socket");
      exit (1);
    }
  }

#ifdef UBSS
  if (npm_proc_online()==BS_ERROR) {
    scm_bscall_error();
    exit (1);
  }
#else
  printf ("Server ready.\n");
#endif


  /* Wait for request and reply with current time */
  for (;;) {
    length = sizeof (synchro_msg);
    cr = soc_receive (soc, (soc_message) &synchro_msg, length, FALSE);
    if ( (cr == sizeof (synchro_msg))
      && (synchro_msg.magic_number == magic_request_value) ) {

      synchro_msg.magic_number = magic_reply_value;
      get_time (&(synchro_msg.server_time));
      length = cr;
      cr = soc_send (soc, (soc_message) &synchro_msg, length);
      if (cr != SOC_OK) {
        perror ("sending reply");
      }
    } else if (cr != SOC_OK) {
      perror ("receiving request");
    } else {
      fprintf (stderr, "Error : wrong request received");
    }
  }
}

