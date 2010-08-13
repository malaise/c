#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


#include "get_line.h"
#include "socket.h"

static void error(const char *msg, const char *arg)
  __attribute__ ((noreturn));
static void error(const char *msg, const char *arg) {
  fprintf(stderr, "ERROR : %s %s\n", msg, arg);
  fprintf(stderr, "Usage : udp_send <dest_type> <dest>:<port> [ <message> ]\n");
  fprintf(stderr, "   <dest_type> ::= lan | host\n");
  fprintf(stderr, "   <dest> ::= <lan_or_host_name> | <lan_or_host_ip_address>\n");
  fprintf(stderr, "   <port> ::= <port_name> | <port_no>\n");
  fprintf(stderr, "Reads stdin if no <message>.\n");
  exit (1);
}

static char message[1024 *1024];

int main (int argc, char *argv[]) {
  boolean dest_lan;
  char host_name[1024];
  soc_host host_no;
  char port_name[256];
  soc_port port_no;
  soc_token soc = init_soc;
  char buffer[1024];
  int i, res, len;
  boolean interactive;

  /* Parse command line arguments */
  /* Syntax is udp_send lan/host <dest>:<port> [ <message> ] */
  if (argc < 3) error("Invalid number of arguments", "");

  if (strcmp(argv[1], "host") == 0) {
    dest_lan = false;
  } else if (strcmp(argv[1], "lan") == 0) {
    dest_lan = true;
  } else {
    error("Invalid argument", argv[1]);
  }
  interactive = (isatty(0) == 1);

  /* Locate ':' in dest:port */
  strcpy (host_name, argv[2]);
  res = -1;
  for (i = 0; i < (int) strlen(host_name); i++) {
    if (host_name[i] == ':') {
      if (res != -1) {
        error ("Invalid destination", host_name);
      }
      res = i;
    }
  }
  /* ':' must exist and not at beginning or end */
  if ( (res == -1) || (res == 0) || (res == (int)strlen (host_name) - 1) ) {
    error ("Invalid destination", host_name);
  }
  host_name[res] = '\0';
  strcpy (port_name, &host_name[res+1]);

  /* Create socket */
  res = soc_open(&soc, udp_socket);
  if (res != SOC_OK) {
      error("Socket creation", soc_error(res));
  }

  /* Set destination */
  if (soc_str2host (host_name, &host_no) == SOC_OK) {
    if (soc_str2port (port_name, &port_no) == SOC_OK) {
      res = soc_set_dest_host_port(soc, &host_no, port_no);
    } else {
      res = soc_set_dest_host_service (soc, &host_no, port_name);
    }
  } else {
    if (soc_str2port (port_name, &port_no) == SOC_OK) {
      res = soc_set_dest_name_port(soc, host_name, dest_lan, port_no);
    } else {
      res = soc_set_dest_name_service(soc, host_name, dest_lan, port_name);
    }
  }
  if (res != SOC_OK) {
    error("Setting destination", soc_error(res));
  }

  /* Link */
  if (soc_str2port (port_name, &port_no) == SOC_OK) {
    res = soc_link_port(soc, port_no);
  } else {
    res = soc_link_service (soc, port_name);
  }
  if (res != SOC_OK) {
    error("Linking to port", soc_error(res));
  }

  /* Build or read message */
  message[0] = '\0';
  if (argc > 3) {
    /* Concatenate arguments separated by spaces */
    for (i = 3; i < argc; i++) {
      if (i != 3) {
        strcat (message, " ");
      }
      strcat (message, argv[i]);
    }
    len = strlen(message);
  } else {
    /* Read from stdin */
    if (interactive) {
      printf ("Enter message to send (end with Ctrl-D): ");
      fflush (stdout);
    }
    len = 0;
    for (;;) {
      /* Read a buffer of data from stdin */
      res = (int) get_text (NULL, buffer, sizeof(buffer));
      if (res == 0) {
        /* End of input flow */
        break;
      }
      /* Concatenate to message */
      memmove (&message[len], buffer, res);
      len += res;
    }
    if (interactive) {
      printf ("\n");
    }
  }

  /* Send */
  res = soc_send (soc, message, (soc_length)len);
  if (res != SOC_OK) {
      error("Sending message", soc_error(res));
  }

  /* Receive */
  res = soc_set_blocking (soc, FALSE);
  sleep (1);
  for (;;) {
    res = soc_receive (soc, message, (soc_length)sizeof(message), TRUE, FALSE);
    if (res > 0) {
      message[res] = '\0';
      printf ("-->%s<\n", message);
    } else {
      break;
    }
  }

  exit (0);
}

