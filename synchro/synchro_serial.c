#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>

#include "sig_util.h"
#include "tty.h"
#include "timeval.h"
#include "gorgy_decode.h"

#define BUFFER_SIZE 1000



extern int fd;
static  unsigned char buffer[BUFFER_SIZE];
static  unsigned int index;

static void store (unsigned char oct) {
  if (index == BUFFER_SIZE - 1) {
    index = 0;
  }
  buffer[index] = oct;
  index ++;
}

/* static void print (char oct) {
 *   if (oct < 32) printf ("'%02X' ", (int)oct);
 *   else printf ("%c ", oct);
 * }
 */

static void decode_and_synchro (void) __attribute__ ((noreturn));
static void decode_and_synchro (void) {
    char        precision;
    struct timeval new_time, curr_time;

    /* Cancel timer */
    (void) arm_timer (ITIMER_REAL , 0, 0, 0);

    /* Decode external clock message */
    if (gorgy_decode ((char*)buffer, &new_time, &precision) < 0) {
        fprintf (stderr, "ERROR. Wrong time format or value.\n");
        exit (2);
    }
    if (precision == '?') {
        fprintf (stderr, "ERROR. Bad precision.\n");
        exit (2);
    } else if (precision == '#') {
        fprintf (stderr, "WARNING. degraded precision.\n");
    } else {
        fprintf (stderr, "INFO. good precision.\n");
    }

    /* For computing delta */
    if (gettimeofday(&curr_time, (struct timezone*) NULL) == -1) {
        perror ("ERROR. Gettimeofday");
        exit (2);
    }

    /* Set time */
    if (settimeofday(&new_time, (struct timezone*) NULL) == -1) {
        perror ("ERROR. Settimeofday");
        exit (2);
    }


    /* Print delta */
    (void) sub_time (&new_time, &curr_time);
    printf ("Synchro %d.%06d s\n", (int)new_time.tv_sec, (int)new_time.tv_usec);
    exit(0);
}

static void sig_handler(int signum) __attribute__ ((noreturn));
static void sig_handler(int signum __attribute__ ((unused)) ) {
    fprintf (stderr, "ERROR. No time received\n");
    exit (2);
}


int main(int argc, char *argv[]) {
  unsigned char oct;

  int started;

  if ( argc != 2) {
    printf ("SYNTAX ERROR. Usage : serial_spy <tty_no>:<stopb>:<datab>:<parity>:<bauds>\n");
    exit (1);
  }

  init_tty(argv[1], 1);

  /* Hook signal handler and start 1 timer (3s) */
  if (set_handler (SIGALRM, sig_handler, NULL) == ERR) {
      fprintf (stderr, "ERROR. Setting signal handler\n");
      exit (2);
  }
  if (arm_timer (ITIMER_REAL , 3, 0, 0) == -1) {
      fprintf (stderr, "ERROR. Starting timer\n");
      exit (2);
  }


  started = 0;
  index = 0;

  for(;;) {
    read_tty (&oct, 1);
    if (oct == 0x02) started = 1;
    if (started) {
      store (oct);
      if (oct == 0x0d) {
        decode_and_synchro();
      }
    }
  }
}

