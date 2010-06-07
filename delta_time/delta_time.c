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

/* Timeout for client to collect replies */
#define DELAY_CLIENT_MS 1000

/* Current program name */
static char prog[256];


/* Prog syntax */
static void usage (void) {
  fprintf (stderr, "Usage: %s <mode> <imp_address> <port_num>\n", prog);
  fprintf (stderr, "  <mode> ::= -s | -c\n");
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

/* Info kept in client about from each server */
typedef struct {
  soc_host host;
  timeout_t server_time;
  timeout_t reception_time;
} info_type;

/* THE MAIN */
int main (const int argc, const char * argv[]) {
  /* Mode client or server */
  boolean client;

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
  char buff[256];
  int res;

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
  if (argc != 4) {
    error ("Invalid arguments");
  }
  /* Parse mode */
  if (strcmp (argv[1], "-c") == 0) {
    client = TRUE;
  } else if (strcmp (argv[1], "-s") == 0) {
    client = FALSE;
  } else {
    sprintf (buff, "Invalid mode %s", argv[1]);
    error (buff);
  }
  /* Parse IPM address and port */
  if (soc_str2host (argv[2], &lan) != SOC_OK) {
    sprintf (buff, "Invalid ipm address %s", argv[2]);
    error (buff);
  }
  if (soc_str2port (argv[3], &port) != SOC_OK) {
    sprintf (buff, "Invalid port num %s", argv[2]);
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
  /* Get start time, init timeout */
  get_time (&start_time);
  /* Client/server report ready */
  if (client) {
    end_time = start_time;
    incr_time (&end_time, DELAY_CLIENT_MS);
    printf ("Client mcasting at address ");
  } else {
    printf ("Server ready at address ");
    end_time.tv_sec = -1;
    end_time.tv_usec = -1;
  }
  buff[0]='\0';
  addr_image (&lan, buff);
  printf ("%s on port %d\n", buff, (int) port);
  /* Client initial ping request */
  if (client) {
    get_time (&start_time);
    msg.ping = TRUE;
    msg.time = start_time;
    if (soc_send (soc, (soc_message) &msg, sizeof(msg)) != SOC_OK) {
      perror ("sending ping");
      error ("Sending ping request failed");
    }
  } 

  /*************/
  /* Main loop */
  /*************/
  /* Init for main loop */
  wait_timeout = end_time;
  /* Main loop */
  for (;;) {
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
        printf ("Aborted.\n");
        break;
      }
    } else if (fd == NO_EVENT) {
      /* Timeout */
      break;
    } else if (fd != soc_fd) {
      sprintf (buff, "Invalid fd %d received", fd);
      error (buff);
    }
    /* Now this is the socket, read message */
    res = soc_receive (soc, (soc_message) &msg, sizeof(msg), TRUE, FALSE);
    if (res < 0) {
      perror ("reading from socket");
      error ("Reading message failed");
    } else if (res != sizeof(msg)) {
      sprintf (buff, "Invalid size received, expected %d, got %d",
                     (int)sizeof(msg), res);
      error (buff);
    }
    get_time (&current_time);
    /* Client stand server different behaviour */
    if (client && !msg.ping) {
      /* Client stores the address and time of server, if pong */
      if (soc_get_dest_host (soc, &(info.host)) != SOC_OK) {
        perror ("getting dest host");
        error ("Getting server address failed");
      }
      info.server_time = msg.time;
      info.reception_time = current_time;
      dlist_insert (&list, &info, TRUE);

    } else if ((!client) && msg.ping) {
      /* Server only replies pong and time to ping */
      msg.time = current_time;
      msg.ping = FALSE;
      msg.time = start_time;
      if (soc_send (soc, (soc_message) &msg, sizeof(msg)) != SOC_OK) {
        perror ("sending pong");
        error ("Sending pong request failed");
      }
    }
    /* Client exits on timeout or goes on waiting */
    if (client) {
      wait_timeout = end_time;
      res = sub_time (&wait_timeout, &current_time);
      if (res <= 0) {
        break;
      }
    }
  } /* End of main loop */

  /* After timeout, client puts info on servers */
  if (client && dlist_length(&list) != 0) {
    dlist_rewind (&list, TRUE);
    do {
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
    } while (dlist_get_pos (&list, FALSE) != 1);
  }

  /* Clean - Close */
  dlist_delete_all (&list);
  (void) evt_del_fd (soc_fd, TRUE);
  (void) soc_close (&soc);
  printf ("Done.\n");
  exit (0);
}

