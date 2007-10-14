#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "tty.h"

int main (int argc, char *argv[]) {

  int kfd, tfd;
  int nfd;
  fd_set fixed_mask, select_mask;
  int cr;
  char c;
  int i;
  int echo;
  int mapda;
  int ctsrts;

  if (argc < 2) {
    fprintf (stderr, "Usage %s <tty_spec> [ echo ] [ crlf ] [ noctsrts ]\n",
                     argv[0]);
    init_tty("", 0, 0, 0);
    exit (1);
  }

  echo = 0;
  mapda = 0;
  ctsrts = 1;
  for (i = 2; i < argc; i++) {
    if (strcmp(argv[i], "echo") == 0) {
      echo = 1;
    } else if (strcmp(argv[i], "crlf") == 0) {
      mapda = 1;
    } else if (strcmp(argv[i], "noctsrts") == 0) {
      ctsrts = 0;
    } else {
      fprintf (stderr, "Usage %s <tty_spec> [ echo ] [ crlf ] [ noctsrts ]\n",
               argv[0]);
      init_tty("", 0, 0, 0);
      exit (1);
    }
  }

  init_tty(argv[1], ctsrts, echo, mapda);
  tfd = get_tty_fd();
  printf ("Ready. Exit with Ctrl X.\n");

  kfd = fileno(stdin);
  init_kbd(kfd);

  nfd = tfd;
  if (nfd < kfd) nfd = kfd;
  FD_ZERO (&fixed_mask);
  FD_SET (tfd, &fixed_mask);
  FD_SET (kfd, &fixed_mask);

  for (;;) {
    memcpy (&select_mask, &fixed_mask, sizeof(fd_set));
    cr = select (nfd+1, &select_mask, (fd_set*)NULL, (fd_set*)NULL,
         (struct timeval*)NULL);
    if (cr == -1) {
      if (errno != EINTR) {
        perror ("select");
      }
    } else if (cr == 0) {
      fprintf (stderr, "select returned 0\n");
    } else {
      if (FD_ISSET(kfd, &select_mask) ) {
#ifdef DEBUG
fprintf (stderr, "kbd event selected\n");
#endif
        errno = 0;
        cr = read (kfd, &c, 1);
        if (cr != 1) {
          perror ("read kbd");
        }
#ifdef DEBUG
fprintf (stderr, "kbd char read: %c 0x%x\n", c, (int)c);
#endif

        if (c == 0x18) {
          /* Exit */
          restore_kbd (kfd);
          restore_tty ();
          (void) putchar ((int)'\n');
          exit (0);
        }

        send_tty (c);
#ifdef DEBUG
fprintf (stderr, "sent to tty.\n");
#endif
      } else if (FD_ISSET(tfd, &select_mask) ) {
#ifdef DEBUG
fprintf (stderr, "tty event selected\n");
#endif
        read_tty (&c);
#ifdef DEBUG
fprintf (stderr, "tty char read: %c %x\n", c, (int)c);
#endif
        (void) putchar ((int)c);
#ifdef DEBUG
fprintf (stderr, "sent to display.\n");
#endif
        (void) fflush (stdout);
      } else {
        fprintf (stderr, "select returned but fd not set\n");
      }
    }
 
  }

}

