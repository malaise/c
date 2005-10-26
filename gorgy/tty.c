#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/termios.h>

#include "tty.h"
#define TTY_DEV "/dev/tty"

#define EXIT { \
  printf ("TTY specification ERROR\n"); \
  printf ("<tty_no>:<stopb>:<datab>:<parity>:<bauds>\n"); \
  printf (" example : 00:1:7:EVEN:9600\n"); \
  exit (1); \
}

int fd;
static char tty_spec[50];
static int start;
static int last;

static void parse (char *arg) {

  int i;

  last = (int)strlen(arg);
  if (last > (int)sizeof(tty_spec)-1) {
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

  if (start == last) {
    start = -1;
  } else {
    start ++;
  }

}



void init_tty (char *arg, int read) {

  char tty_name[50];
  struct termio mode;	

  unsigned short c_flags;

  parse (arg);

  strcpy (tty_name, TTY_DEV);
  strcat (tty_name, &tty_spec[start]);

  next (); if (start == -1) EXIT;
  if      (strcmp(&tty_spec[start], "1") == 0) c_flags = 0;
  else if (strcmp(&tty_spec[start], "2") == 0) c_flags = CSTOPB;
  else    EXIT;


  next (); if (start == -1) EXIT;
  if      (strcmp(&tty_spec[start], "7") == 0) c_flags |= CS7;
  else if (strcmp(&tty_spec[start], "8") == 0) c_flags |= CS8;
  else    EXIT;

  next (); if (start == -1) EXIT;
  if      (strcmp(&tty_spec[start], "NONE") == 0) ;
  else if (strcmp(&tty_spec[start],  "ODD") == 0) c_flags |= (PARENB | PARODD);
  else if (strcmp(&tty_spec[start], "EVEN") == 0) c_flags |= PARENB;
  else    EXIT;

  next (); if (start == -1) EXIT;
  if      (strcmp(&tty_spec[start],   "300") == 0) c_flags |= B300;
  else if (strcmp(&tty_spec[start],   "600") == 0) c_flags |= B600;
  else if (strcmp(&tty_spec[start],  "1200") == 0) c_flags |= B1200;
  else if (strcmp(&tty_spec[start],  "1800") == 0) c_flags |= B1800;
  else if (strcmp(&tty_spec[start],  "2400") == 0) c_flags |= B2400;
  else if (strcmp(&tty_spec[start],  "4800") == 0) c_flags |= B4800;
  else if (strcmp(&tty_spec[start],  "9600") == 0) c_flags |= B9600;
  else if (strcmp(&tty_spec[start], "19200") == 0) c_flags |= B19200;
  else if (strcmp(&tty_spec[start], "38400") == 0) c_flags |= B38400;

  next (); if (start != -1) EXIT;

  if (read) {
    fd = open (tty_name, O_RDONLY);
  } else {
    fd = open (tty_name, O_WRONLY);
  }
  if (fd < 0) {
    perror ("Error open");
    fprintf (stderr, ">%s<\n", tty_name);
    exit(-1);
  }

  if (ioctl (fd, TCGETA, (char*) &mode) <0) {
    perror ("Error ioctl get");
    exit (-1);
  }
  mode.c_iflag = 0;
  mode.c_oflag = 0;
  mode.c_lflag = 0;
  mode.c_cflag = (CLOCAL | HUPCL | c_flags);
  if (read) {
    mode.c_cflag = (mode.c_cflag | CREAD);
  }
  mode.c_cc[VMIN] = 1;
  mode.c_cc[VTIME] = 0;

  if (ioctl (fd, TCSETA, (char*) &mode) <0) {
    perror ("Error ioctl set");
    exit (-1);
  }

  printf ("TTY %s initialised.\n", tty_name);

}


void send_tty (char *msg, int len) {

int cr;

  for (;;) {
    cr = (write (fd, msg, len) >= 0);
    if (cr) return;
    if (errno != EINTR) {
      perror ("write");
      return;
    }
  }
}

void read_tty (void *buffer, int nbre_octet) {
int i;
char *p;
int res;

  for (i=0, p=(char*)buffer; i<nbre_octet; i++, p++) {
    do {
      res = read (fd, p, 1);
      if ( (res == -1)  && (errno != EINTR) )  {
        perror ("read");
      }
    } while (res != 1);
  }
}
