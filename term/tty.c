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

static struct termio kbd_mode;	
static int fd = -1;
static char tty_spec[50];
static int start;
static int last;

static void parse (const char *arg) {
int i;
  last = strlen(arg);
  if (last > (int)sizeof(tty_spec)-1) {
    EXIT;
  }
  strcpy (tty_spec, arg);
  for (i=0; i<last; i++) {
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

  if (start == last) {
    start = -1;
  } else {
    start ++;
  }

}



void init_tty (const char *arg, int ctsrts, int echo, int crlf) {

  char tty_name[50];
  struct termio mode;

  unsigned short c_flags;

  parse (arg);

  strcpy (tty_name, TTY_DEV);
  strcat (tty_name, &tty_spec[start]);

  next (); if (start == -1) EXIT;
  c_flags = CLOCAL;
  if (ctsrts) {
    c_flags |= CRTSCTS;
  } 

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
  if      (strcmp(&tty_spec[start],    "300") == 0) c_flags |= B300;
  else if (strcmp(&tty_spec[start],    "600") == 0) c_flags |= B600;
  else if (strcmp(&tty_spec[start],   "1200") == 0) c_flags |= B1200;
  else if (strcmp(&tty_spec[start],   "1800") == 0) c_flags |= B1800;
  else if (strcmp(&tty_spec[start],   "2400") == 0) c_flags |= B2400;
  else if (strcmp(&tty_spec[start],   "4800") == 0) c_flags |= B4800;
  else if (strcmp(&tty_spec[start],   "9600") == 0) c_flags |= B9600;
  else if (strcmp(&tty_spec[start],  "19200") == 0) c_flags |= B19200;
  else if (strcmp(&tty_spec[start],  "38400") == 0) c_flags |= B38400;
  else if (strcmp(&tty_spec[start], "115200") == 0) c_flags |= B115200;
  else if (strcmp(&tty_spec[start], "230400") == 0) c_flags |= B230400;
  else    EXIT;

  next (); if (start != -1) EXIT;

  fd = open (tty_name, O_RDWR | O_NDELAY);
  if (fd < 0) {
    perror ("Error open");
    fprintf (stderr, ">%s<\n", tty_name);
    exit(1);
  }

  if (flock (fd, LOCK_EX | LOCK_NB) != 0) {
    perror ("Error flock");
    fprintf (stderr, "Device %s is locked.\n", tty_name);
    close (fd);
    exit(1);
  }

  if (ioctl (fd, TCGETA, (char*) &mode) <0) {
    perror ("Error ioctl get");
    restore_tty ();
    exit (1);
  }
  if (crlf) {
    mode.c_oflag = OCRNL;
    mode.c_iflag = ICRNL;
  } else {
    mode.c_oflag = 0;
    mode.c_iflag = 0;
   }
  if (echo) {
    mode.c_lflag = ECHO;
  } else {
    mode.c_lflag = 0;
  }
  if (!ctsrts) {
    mode.c_iflag |= IXOFF | IXON;
  }
  mode.c_cflag = c_flags;
  mode.c_cc[VMIN] = 1;
  mode.c_cc[VTIME] = 0;

  if (ioctl (fd, TCSETA, (char*) &mode) <0) {
    perror ("Error ioctl set");
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
      perror ("write");
      return;
    }
  }
}

void read_tty (char *c) {
int res;

  do {
    res = read (fd, c, 1);
    if ( (res == -1)  && (errno != EINTR) )  {
      perror ("read");
    }
  } while (res != 1);
}

int get_tty_fd(void) {
  return (fd);
}


void init_kbd (int kfd) {
  struct termio mode;

  if (ioctl (kfd, TCGETA, (char*) &mode) <0) {
    perror ("Error ioctl get");
    restore_tty ();
    exit (1);
  }
  memcpy (&kbd_mode, &mode, sizeof(struct termio));
  mode.c_iflag = 0;
  mode.c_oflag = 0;
  mode.c_lflag = 0;
  mode.c_cflag = (CLOCAL);
  mode.c_cc[VMIN] = 1;
  mode.c_cc[VTIME] = 0;

  if (ioctl (kfd, TCSETA, (char*) &mode) <0) {
    perror ("Error ioctl set");
    restore_tty ();
    exit (1);
  }

#ifdef DEBUG
fprintf (stderr, "KBD initialised.\n");
#endif

}

void restore_kbd (int kfd) {
  struct termio mode;

  memcpy (&mode, &kbd_mode, sizeof(struct termio));
  if (ioctl (kfd, TCSETA, (char*) &mode) <0) {
    perror ("Error ioctl set");
    restore_tty ();
    exit (1);
  }
}

