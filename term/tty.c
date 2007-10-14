#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/termios.h>
#include <sys/file.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#include "tty.h"

#define TTY_DEV "/dev/"

#define EXIT { \
  printf ("TTY specification ERROR\n"); \
  printf ("<tty_name>:<datab>:<parity>:<stopb>:<bauds>\n"); \
  printf (" example : tty00:8:N:1:9600\n"); \
  exit (1); \
}

static struct termios kbd_mode;	
static int fd = -1;
static char tty_spec[50];
static int start;
static unsigned int last;

static void parse (const char *arg) {
  unsigned int i;
  last = strlen(arg);
  if (last > sizeof(tty_spec)-1) {
    EXIT;
  }
  strcpy (tty_spec, arg);
  for (i = 0; i < last; i++) {
    if (tty_spec[i] == ':') {
      tty_spec[i] = '\0';
    }
  }
  start = 0;
}

static void next (void) {
  do {
    start++;
  } while (tty_spec[start]!='\0' );

  if ((unsigned)start == last) {
    start = -1;
  } else {
    start ++;
  }

}

void init_tty (const char *arg, int ctsrts, int echo, int crlf) {

  char tty_name[50];
  struct termios mode;

  tcflag_t c_flags;
  speed_t speed;
  long flags;

  parse (arg);

  strcpy (tty_name, TTY_DEV);
  strcat (tty_name, &tty_spec[start]);

  /* Parse tty spec and set c_flags and speeds */
  c_flags = 0;
  next (); if (start == -1) EXIT;

  if      (strcmp(&tty_spec[start], "7") == 0) c_flags |= CS7;
  else if (strcmp(&tty_spec[start], "8") == 0) c_flags |= CS8;
  else    EXIT;

  next (); if (start == -1) EXIT;
  if      (strcmp(&tty_spec[start], "N") == 0) ;
  else if (strcmp(&tty_spec[start], "O") == 0) c_flags |= (PARENB | PARODD);
  else if (strcmp(&tty_spec[start], "E") == 0) c_flags |= PARENB;
  else    EXIT;

  next (); if (start == -1) EXIT;
  if      (strcmp(&tty_spec[start], "1") == 0);
  else if (strcmp(&tty_spec[start], "2") == 0) c_flags = CSTOPB;
  else    EXIT;

  next (); if (start == -1) EXIT;
  if      (strcmp(&tty_spec[start],    "300") == 0) speed = B300;
  else if (strcmp(&tty_spec[start],    "600") == 0) speed = B600;
  else if (strcmp(&tty_spec[start],   "1200") == 0) speed = B1200;
  else if (strcmp(&tty_spec[start],   "1800") == 0) speed = B1800;
  else if (strcmp(&tty_spec[start],   "2400") == 0) speed = B2400;
  else if (strcmp(&tty_spec[start],   "4800") == 0) speed = B4800;
  else if (strcmp(&tty_spec[start],   "9600") == 0) speed = B9600;
  else if (strcmp(&tty_spec[start],  "19200") == 0) speed = B19200;
  else if (strcmp(&tty_spec[start],  "38400") == 0) speed = B38400;
  else if (strcmp(&tty_spec[start], "115200") == 0) speed = B115200;
  else if (strcmp(&tty_spec[start], "230400") == 0) speed = B230400;
  else    EXIT;

  next (); if (start != -1) EXIT;

  fd = open (tty_name, O_RDWR | O_NONBLOCK | O_NOCTTY);
  if (fd < 0) {
    perror ("Error open tty");
    fprintf (stderr, ">%s<\n", tty_name);
    exit(1);
  }

  if (flock (fd, LOCK_EX | LOCK_NB) != 0) {
    perror ("Error flock tty");
    fprintf (stderr, "Device %s is locked.\n", tty_name);
    close (fd);
    exit(1);
  }

  flags = fcntl (fd, F_GETFL);
  if (flags == -1) {
    perror ("Error fcntl getfl tty");
    fprintf (stderr, ">%s<\n", tty_name);
    exit(1);
  }
  flags = O_RDWR;
  if (fcntl (fd, F_SETFL, flags) != 0) {
    perror ("Error fcntl setfl tty");
    fprintf (stderr, ">%s<\n", tty_name);
    exit(1);
  }


  if (tcgetattr (fd, &mode) < 0) {
    perror ("Error tcgetattr tty");
    restore_tty ();
    exit (1);
  }

  /* Set Serial line charactistics */
  mode.c_cflag = c_flags;
  if (cfsetispeed (&mode, speed) < 0) {
    perror ("Error cfsetispeed");
    fprintf (stderr, ">%s<\n", tty_name);
    exit(1);
  }
  if (cfsetospeed (&mode, speed) < 0) {
    perror ("Error cfsetospeed");
    fprintf (stderr, ">%s<\n", tty_name);
    exit(1);
  }

  if (crlf) {
    mode.c_oflag = OCRNL;
    mode.c_iflag = ICRNL;
  } else {
    mode.c_oflag = 0;
    mode.c_iflag = 0;
  }
  mode.c_iflag |= IGNBRK;
  if (echo) {
    mode.c_lflag = ECHO;
  } else {
    mode.c_lflag = 0;
  }
   mode.c_cflag |= CLOCAL | CREAD;
  if (ctsrts) {
    mode.c_cflag |= CRTSCTS;
  } else {
    mode.c_cflag &= ~CRTSCTS;
    mode.c_iflag |= IXOFF | IXON;
  }
  mode.c_cc[VMIN] = 1;
  mode.c_cc[VTIME] = 5;

  if (tcsetattr (fd, TCSANOW, &mode) < 0) {
    perror ("Error tcsetattr tty");
    restore_tty ();
    exit (1);
  }
#ifdef DEBUG
fprintf (stderr, "TTY %s initialised.\n", tty_name);
#endif

}

void restore_tty (void) {
  if (fd == -1) return;
  (void) flock (fd, LOCK_UN);
  (void) close (fd);
}

void send_tty (char c) {

  int cr;

  for (;;) {
    cr = (write (fd, &c, 1) >= 0);
    if (cr) return;
    if (errno != EINTR) {
      perror ("write tty");
      return;
    }
  }
}

void read_tty (char *c) {
  int res;

  do {
    res = read (fd, c, 1);
    if ( (res == -1)  && (errno != EINTR) )  {
      perror ("read tty");
    }
  } while (res != 1);
}

int get_tty_fd(void) {
  return (fd);
}


void init_kbd (int kfd) {
  struct termios mode;

  if (tcgetattr (kfd, &mode) < 0) {
    perror ("Error tcgetattr kbd");
    restore_tty ();
    exit (1);
  }
  memcpy (&kbd_mode, &mode, sizeof(struct termios));
  mode.c_iflag = 0;
  mode.c_oflag = 0;
  mode.c_lflag = 0;
  mode.c_cflag = (CLOCAL);
  mode.c_cc[VMIN] = 1;
  mode.c_cc[VTIME] = 0;

  if (tcsetattr (kfd, TCSANOW, &mode) < 0) {
    perror ("Error tcsetattr kbd");
    restore_tty ();
    exit (1);
  }

#ifdef DEBUG
fprintf (stderr, "KBD initialised.\n");
#endif

}

void restore_kbd (int kfd) {

  if (tcsetattr (kfd, TCSANOW, &kbd_mode) < 0) {
    perror ("Error tcsetattr kbd");
    restore_tty ();
    exit (1);
  }
}

