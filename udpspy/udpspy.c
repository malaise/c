#include "socket.h"
#include "wait_evt.h"
#include "util.h"

/* Message read */
static char message[128*1024];

int main (const int argc, const char *argv[]) {
  /* Result of operation */
  int res;
  char buffer[255];

  /* The socket and its fd */
  soc_token socket = init_soc;
  int fd;

  /* Event result */
  boolean read;
  int timeout;
  int evtfd;

  /* parse arguments */
  parse_args (argc, argv);

  /* Create socket and get fd */
  if ( (res = soc_open (&socket, udp_socket)) != SOC_OK) {
    trace ("soc_open error", soc_error (res));
    error ("cannot open socket", "");
  }
  if ( (res = soc_get_id (socket, &fd)) != SOC_OK) {
    trace ("soc_get_id error", soc_error (res));
    error ("cannot get socket fd", "");
  }
  /* Bind socket to lan:port */
  bind_socket (socket);

  /* Attach fd for reading */
  if ( (res = evt_add_fd (fd, TRUE)) != OK) {
    trace ("evt_add_fd error", "");
    error ("cannot add fd", "");
  }

  /* Main loop */
  timeout = -1;
  for (;;) {
    /* Infinite wait for events */
    if ( (res = evt_wait (&evtfd, & read, &timeout)) != OK) {
      trace ("evt_wait error", "");
      error ("cannot wait for event", "");
    }
    /* Analyse event */
    if (evtfd == SIG_EVENT) {
      if (get_signal() == SIG_TERMINATE) {
        /* Sigterm/sigint */
        break;
      } /* else unexpected signal => drop */
    } else if (evtfd == fd) {
      /* Got a packet: read it */
      res = soc_receive (socket, message, sizeof(message), TRUE);
      if (res < 0) {
        sprintf (buffer, "%d", res);
        trace ("soc_receive error", soc_error (res));
        error ("cannot read message", soc_error(res));
      } else {
        if (res > (int)sizeof(message)) {
          sprintf (buffer, "%d", res);
          trace ("soc_receive truncated message length", "buffer");
          res = (int)sizeof(message);
         }
         /* Put message info */
         display (socket, message, res);
      }
    } else if (evtfd >= 0) {
      /* Unexpected event on an unexpected fd */
      sprintf (buffer, "%d", evtfd);
      trace ("evt_wait got unexpected even on fd", "buffer");
      error ("even on unexpected fd", "");
    }

  } /* Main loop */

  /* Done */
  (void) evt_del_fd (fd, TRUE);
  (void) soc_close (&socket);
  the_end ();
}

