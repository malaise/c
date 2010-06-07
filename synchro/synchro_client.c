#include <stdio.h>
#include <stdlib.h>
#include "socket.h"
#include "timeval.h"
#include "wait_evt.h"

#include "synchro.h"

#define DEFAULT_TIMEOUT_MS   1000
#define DEFAULT_ACCURACY_MS   100

#define USAGE() {fprintf(stderr, "Usage : synchro_client <port_name> [ <timeout_ms> [ <accuracy_ms> ]\n");}

int main (int argc, char *argv[]) {

  timeout_t timeout;
  int timeout_ms;
  int accuracy_ms;
  soc_token soc = init_soc;
  soc_port port_no;
  char lan_name[80];
  synchro_msg_t synchro_msg;
  soc_length length;
  int cr, fd;
  boolean read;
  unsigned int travel_ms;

  timeout_t request_time, reply_time, travel_delta;
  timeout_t accuracy_timeout, wait_timeout;


  if ( (argc != 2) && (argc != 3) && (argc != 4) ) {
    fprintf (stderr, "SYNTAX error : Wrong number of arguments\n");
    USAGE();
    exit (1);
  }

  if (argc >= 3) {
    timeout_ms = atoi(argv[2]);
    if (timeout_ms <= 0) {
      fprintf (stderr, "SYNTAX error : Wrong timeout value\n");
      USAGE();
      exit (1);
    }
  } else {
    timeout_ms = DEFAULT_TIMEOUT_MS;
  }
  wait_timeout.tv_sec = timeout_ms / 1000; 
  wait_timeout.tv_usec = (timeout_ms % 1000) * 1000;

  if (argc == 4) {
    accuracy_ms = atoi(argv[3]);
    if (accuracy_ms <= 0) {
      fprintf (stderr, "SYNTAX error : Wrong accuracy value\n");
      USAGE();
      exit (1);
    }
  } else {
    accuracy_ms = DEFAULT_ACCURACY_MS;
  }
  accuracy_timeout.tv_sec = accuracy_ms / 1000;
  accuracy_timeout.tv_usec = (accuracy_ms % 1000) * 1000;


  if (soc_get_lan_name(lan_name, sizeof(lan_name)) != SOC_OK) {
    perror ("getting lan name");
    exit (1);
  }

  if (soc_open(&soc, udp_socket) != SOC_OK) {
    perror ("opening socket");
    exit (1);
  }

  port_no = atoi(argv[1]);
  if (port_no <= 0) {
    if (soc_set_dest_name_service(soc, lan_name, true, argv[1]) != SOC_OK) {
      perror ("setting destination service");
      exit (1);
    }
  } else {
    if (soc_set_dest_name_port(soc, lan_name, true, port_no) != SOC_OK) {
      perror ("setting destination port");
      exit (1);
    }
  }

  if (soc_get_dest_port(soc, &port_no) != SOC_OK) {
    perror ("getting destination port no");
    exit (1);
  }

  if (soc_link_port(soc, port_no) != SOC_OK) {
    perror ("linking socket");
    exit (1);
  }

  /* List for wait */
  if (soc_get_id(soc, &fd) != SOC_OK) {
    perror ("getting socket id");
    exit (1);
  }
  if (evt_add_fd(fd, TRUE) != WAIT_OK) {
    perror("Adding fd");
    exit (1);
  }

  for (;;) {
    synchro_msg.magic_number = magic_request_value;
    get_time (&(synchro_msg.request_time));

    cr = soc_send (soc, (soc_message) &synchro_msg, sizeof(synchro_msg));
    if (cr != SOC_OK) {
      perror ("sending request");
      exit (1);
    }

    request_time = synchro_msg.request_time;

    for (;;) {

      timeout = wait_timeout;
      if (evt_wait (&fd, &read, &timeout) != WAIT_OK) {
        perror ("waiting for event");
        exit (1);
      }

      if (fd == NO_EVENT) {
        printf ("Timeout expired. Giving up.\n");
        exit (2);
      } else if (fd <= 0) {
        /* Other non fd event */
        continue;
      }

      length = sizeof (synchro_msg);
      cr = soc_receive (soc, (soc_message) &synchro_msg, length, FALSE, FALSE);
      get_time (&reply_time);

      if ( cr == sizeof (synchro_msg) ) {
        if (synchro_msg.magic_number == magic_request_value) {
          ;
        } else if ( (synchro_msg.magic_number == magic_reply_value)
                 && (synchro_msg.request_time.tv_sec == request_time.tv_sec)
                 && (synchro_msg.request_time.tv_usec == request_time.tv_usec) ) {

          travel_delta = reply_time;
          (void) sub_time (&travel_delta, &request_time);
          if (comp_time(&travel_delta, &accuracy_timeout) > 0) {
              printf ("Insuficient accuracy. Skipping...\n");
              break;
          }

          /* Compute time (reply + travel/2) and settimofday */
          travel_ms = travel_delta.tv_sec * 1000;
          travel_ms += travel_delta.tv_usec / 1000;
          incr_time (&synchro_msg.server_time, travel_ms / 2);

          (void) sub_time (&reply_time, &synchro_msg.server_time);
          printf ("Synchro %d.%06d s\n", (int)reply_time.tv_sec,
                                         (int)reply_time.tv_usec);

          if (settimeofday (&synchro_msg.server_time, NULL) < 0) {
             perror ("settimeofday");
             exit (1);
          }
          exit (0);
        } else {
          fprintf (stderr, "Error : wrong reply received");
        }

      } else if (cr != SOC_OK) {
        perror ("receiving reply");
      } else {
        fprintf (stderr, "Error : wrong reply received");
      }
    }

    /* Reset dest */
    if (soc_set_dest_name_port(soc, lan_name, true, port_no) != SOC_OK) {
      perror ("linking socket");
      exit (1);
    }
  }
}

