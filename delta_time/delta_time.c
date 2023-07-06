#include <stdlib.h>
#include <stdio.h>
#include <libgen.h>
#include <string.h>
#include <errno.h>

#include "boolean.h"
#include "timeval.h"
#include "socket.h"
#include "wait_evt.h"
#include "dynlist.h"

/* Distributed algorythm:                                                     */
/* The first process starts as client and:                                    */
/*  - Sends a ping request (that will be lost)                                */
/*  - Waits 1s                                                                */
/*  - Becomes server: It will only reply to ping requests                     */
/* A second process starts as client: it sends the ping request and waits 1s  */
/* Meanwhile, each server replies to the client by providing its local time   */
/* The client collects these replies and computes the shows the delta of time */
/*  of each server                                                            */
/* After the timeout, the client shows the delta of time of each server, then */
/*  in turn it becomes server                                                 */

/* Timeout for client to collect replies */
#define DELAY_CLIENT_MS 1000

/* Current program name */
static char prog[256];


/* Prog syntax */
static void usage (void) {
  fprintf (stderr, "Usage: %s <imp_address>:<port_num>\n", prog);
}

/* Log error messages and exit */
static void error (const char * msg) __attribute__ ((noreturn));
static void error (const char * msg) {
  fprintf (stderr, "ERROR: %s.\n", msg);
  usage();
  exit (1);
}

/* Put IP address image */
static void addr_image (soc_host *addr, char image[]) {
  int i;
  char buf[10];
  for (i = 0; i < 4; i++) {
    sprintf (buf, "%d", (int)addr->bytes[i]);
    strcat (image, buf);
    if (i != 3) {
      strcat (image, ".");
    }
  }
}


/* Message exchanged on IPM socket */
typedef struct {
  boolean ping;
  timeout_t time;
} msg_type;

/* Info kept in client about each server */
typedef struct {
  soc_host host;
  timeout_t server_time;
  timeout_t reception_time;
} info_type;

/* THE MAIN */
int main (const int argc, const char * argv[]) {

  /* Socket data */
  soc_token soc = init_soc;
  soc_host lan;
  soc_port port;
  int soc_fd, fd;

  /* Socket message */
  msg_type msg;

  /* Times and timeouts */
  timeout_t start_time, end_time, current_time;
  timeout_t wait_timeout;
  double local_time, remote_time;

  /* Dynamic list of server infos */
  dlist list;
  info_type info;

  /* Utilities */
  boolean for_read;
  char addr[256], buff[1024];
  int res;
  char *index;

  /*********/
  /* Start */
  /*********/
  /* Save prog name */
  strcpy (prog, argv[0]);
  strcpy (prog, basename (prog));

  /*******************/
  /* Parse arguments */
  /*******************/
  /* Check args */
  if (argc != 2) {
    error ("Invalid argument");
  }
  /* Parse IPM address and port */
  strcpy (addr, argv[1]);
  index = strstr (addr, ":");
  if (index == NULL) {
     error ("Invalid argument");
  }
  *index = '\0';
  index++;
  if (soc_str2host (addr, &lan) != SOC_OK) {
    sprintf (buff, "Invalid ipm address %s", addr);
    error (buff);
  }
  if (soc_str2port (index, &port) != SOC_OK) {
    sprintf (buff, "Invalid port num %s", index);
    error (buff);
  }

  /**************/
  /* Initialize */
  /**************/
  /* Init dynamic list */
  dlist_init (& list, sizeof(info_type));
  /* Init socket */
  if (soc_open (&soc, udp_socket) != SOC_OK) {
    perror ("opening socket");
    error ("Socket initialization failed");
  }
  if (soc_set_dest_host_port (soc, &lan, port) != SOC_OK) {
    perror ("setting destination");
    error ("Socket initialization failed");
  }
  if (soc_link_port (soc, port) != SOC_OK) {
    perror ("linking to port");
    error ("Socket initialization failed");
  }
  if (soc_get_dest_host (soc, &lan) != SOC_OK) {
    perror ("getting dest lan");
    error ("Socket initialization failed");
  }
  if (soc_get_dest_port (soc, &port) != SOC_OK) {
    perror ("getting dest port");
    error ("Socket initialization failed");
  }
  /* Add socket to waiting point */
  if (soc_get_id(soc, &soc_fd) != SOC_OK) {
    perror ("getting socket id");
    error ("Socket initialization failed");

  }
  if (evt_add_fd(soc_fd, TRUE) != WAIT_OK) {
    perror("Adding fd");
    error ("Socket initialization failed");
  }
  /* Activate signal catching */
  activate_signal_handling();
  /* Report starting */
  buff[0]='\0';
  addr_image (&lan, buff);
  printf ("%s mcasting at address %s on port %d.\n", prog, buff, (int) port);
  /* Init times */
  get_time (&start_time);
  current_time = start_time;
  end_time = start_time;
  incr_time (&end_time, DELAY_CLIENT_MS);
  /* Send initial ping request */
  msg.ping = TRUE;
  msg.time = start_time;
  if (soc_send (soc, (soc_message) &msg, sizeof(msg)) != SOC_OK) {
    perror ("sending ping");
    error ("Sending ping request failed");
  }

  /*************/
  /* Main loop */
  /*************/
  for (;;) {
    /* First step is to loop until timeout */
    if (wait_timeout.tv_sec != -1) {
      wait_timeout = end_time;
      res = sub_time (&wait_timeout, &current_time);
      if (res <= 0) {
        break;
      }
    }
    if (evt_wait (&fd, &for_read, &wait_timeout) != WAIT_OK) {
      perror ("waiting for event");
      error ("Waiting for events failed");
    }
    if (! for_read) {
      error ("Write event received");
    }
    /* Termination signal */
    if (fd == SIG_EVENT) {
      if (get_signal () == SIG_TERMINATE) {
        break;
      }
    } else if (fd == NO_EVENT) {
      /* Timeout: first step ends with a dump of servers */
      if (dlist_length(&list) != 0) {
        dlist_rewind (&list, TRUE);
        for (;;) {
          dlist_read (&list, &info);
          /* Get host name if possible, else dump address */
          res = soc_host_name_of (&info.host, buff, sizeof(buff));
          if (res != SOC_OK) {
            buff[0]='\0';
            addr_image (&info.host, buff);
          }
          /* Compute (Start_time + Reception_time) / 2 */
          local_time = (time_to_double (&start_time)
                        + time_to_double (&info.reception_time) ) / 2.0;
          remote_time = time_to_double (&info.server_time);
          printf ("Host %s is shifted by %4.03fs\n", buff, remote_time - local_time);

          /* Done when last record has been put */
          if (dlist_get_pos (&list, FALSE) == 1) {
            break;
          }
          dlist_move (&list, TRUE);
        }
      }
      /* Now entering second step: infinite timeout */
      wait_timeout.tv_sec = -1;
      wait_timeout.tv_usec = -1;
      printf ("%s ready.\n", prog);
    } else if (fd != soc_fd) {
      sprintf (buff, "Invalid fd %d received", fd);
      error (buff);
    } else {
      /* Now this is the socket, read message, set for reply */
      res = soc_receive (soc, (soc_message) &msg, sizeof(msg), TRUE);
      if (res < 0) {
        perror ("reading from socket");
        error ("Reading message failed");
      } else if (res != sizeof(msg)) {
        sprintf (buff, "Invalid size received, expected %d, got %d",
                       (int)sizeof(msg), res);
        error (buff);
      }
      get_time (&current_time);
      /* Client and server different behaviours */
      if ((wait_timeout.tv_sec != -1) && !msg.ping) {
        /* First step: store the address and time of server, if pong */
        if (soc_get_dest_host (soc, &(info.host)) != SOC_OK) {
          perror ("getting dest host");
          error ("Getting server address failed");
        }
        info.server_time = msg.time;
        info.reception_time = current_time;
        dlist_insert (&list, &info, TRUE);

      } else if ( (wait_timeout.tv_sec == -1) && msg.ping) {
        /* Second step: reply pong and time to the pingging host */
        msg.time = current_time;
        msg.ping = FALSE;
        if (soc_send (soc, (soc_message) &msg, sizeof(msg)) != SOC_OK) {
          perror ("sending pong");
          error ("Sending pong request failed");
        }
      }
    }
  } /* End of main loop */


  /* Clean - Close */
  dlist_delete_all (&list);
  (void) evt_del_fd (soc_fd, TRUE);
  (void) soc_close (&soc);
  printf ("Done.\n");
  exit (0);
}

