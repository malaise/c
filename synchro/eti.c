#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>

#include "boolean.h"
#include "tty.h"
#include "timeval.h"
#include "gorgy_decode.h"
#include "adjtime_call.h"
#include "sig_util.h"

#define BUFFER_SIZE 1000


#define MIN_THRESHOLD_MSEC 10
#define DEFAULT_THRESHOLD_MSEC 100

extern int fd;
static unsigned char buffer[BUFFER_SIZE];
static unsigned int eti_index;
static boolean verbose;
static int threshold_msec;
static boolean frame_expected, frame_received;
static char prev_precision;

static void store (unsigned char oct) {
  if (eti_index == BUFFER_SIZE - 1) {
    eti_index = 0;
  }
  buffer[eti_index] = oct;
  eti_index ++;
}

/*
  static void print (char oct) {
    if (oct < 32) printf ("'%02X' ", (int)oct);
    else printf ("%c ", oct);
  }
*/

static void decode_and_synchro (void) {
  struct timeval new_time, curr_time, curr_delta, curr_adjust, delta_delta;
  char *curr_time_str;
  char precision;
  boolean prev_frame_received;

  prev_frame_received = frame_received;

  /* Get current time ASAP */
  if (gettimeofday(&curr_time, (struct timezone*) NULL) == -1) {
    perror ("Gettimeofday");
    frame_received = true;
    return;
  }

  /* For dating report messages */
  curr_time_str = ctime((time_t*) &curr_time.tv_sec);
  /* Skip last \n */
  curr_time_str[strlen(curr_time_str)-1] = '\0';

  /* A frame after no frame */
  if (!prev_frame_received) {
    printf ("At %s, INFO: time received\n", curr_time_str);
  }

  /* Decode frame */
  if (gorgy_decode ((char*)buffer, &new_time, &precision) < 0) {
    fprintf (stderr, "At %s, ", curr_time_str);
    fprintf (stderr, "ERROR. Wrong time format or value.\n");
    frame_received = false;
    return;
  }

  /* Discard if bad precision. Warn if degraded precision */
  if (precision == '?') {
    if (prev_precision != precision) {
      fprintf (stderr, "At %s, ", curr_time_str);
      fprintf (stderr, "ERROR. Bad precision.\n");
    }
    prev_precision = precision;
    frame_received = true;
    return;
  } else if (precision == '#') {
    if (verbose && (prev_precision != precision)) {
      fprintf (stderr, "At %s, ", curr_time_str);
      fprintf (stderr, "WARNING. Degraded precision.\n");
    }
  } else {
    if ((prev_precision == '?') || (verbose && (prev_precision =='#') ) ) {
      fprintf (stderr, "At %s, ", curr_time_str);
      fprintf (stderr, "INFO. Good precision.\n");
    }
  }
  prev_precision = precision;
  frame_received = true;

  /* Compute delta = new_time - current_time */
  (void) memcpy (&curr_delta, &new_time, sizeof(curr_delta));
  (void) sub_time (&curr_delta, &curr_time);

  /* Get current adjustment  = current_time - desired_time */
  if (adjtime_call ((struct timeval*)NULL, &curr_adjust) == -1) {
    fprintf (stderr, "At %s, ", curr_time_str);
    perror ("Adjtime_call");
    return;
  }

  /* Delta_delta = curr_delta - curr_adjust */
  (void) memcpy (&delta_delta, &curr_delta, sizeof(curr_delta));
  (void) sub_time (&delta_delta, &curr_adjust);

  /* If |delta_delta| < threshold, then */
  if ( (delta_delta.tv_sec != 0) || (abs(delta_delta.tv_usec)/1000 > threshold_msec) ) {
    if (adjtime_call (&curr_delta, (struct timeval*)NULL)  == -1) {
      fprintf (stderr, "At %s, ", curr_time_str);
      perror ("Adjtime_call");
      return;
    }
    if (verbose) {
      printf ("At %s,      Synchro: %d.%06d,      Prev adjust:  %d.%06d\n",
               curr_time_str, (int)curr_delta.tv_sec, (int)curr_delta.tv_usec,
               (int)curr_adjust.tv_sec, (int)curr_adjust.tv_usec);
    }
  }
}

static void sig_handler(int signum __attribute__ ((unused)) ) {
    struct timeval curr_time;
    char *curr_time_str;

    /* Get current time */
    if (gettimeofday(&curr_time, (struct timezone*) NULL) == -1) {
        perror ("Gettimeofday");
        return;
    }

    /* For dating report messages */
    curr_time_str = ctime((time_t*) &curr_time.tv_sec);
    /* Skip last \n */
    curr_time_str[strlen(curr_time_str)-1] = '\0';

    if (frame_expected) {
        fprintf (stderr, "At %s, ", curr_time_str);
        fprintf (stderr, "ERROR. No time received\n");
    }

    /* For tracing when next message arrives */
    frame_received = false;
}


#define USAGE  "Usage : eti <tty_no>:<stopb>:<datab>:<parity>:<bauds> [ <accuracy_ms> ] [ -v ]"

int main(int argc, char *argv[]) {
  unsigned char oct;

  boolean started;

  if (argc == 2) {
    threshold_msec = DEFAULT_THRESHOLD_MSEC;
    verbose = false;
  } else if (argc == 3) {
    if (strcmp(argv[2], "-v") == 0) {
      threshold_msec = DEFAULT_THRESHOLD_MSEC;
      verbose = true;
    } else {
      threshold_msec = atoi(argv[2]);
    }
  } else if ( (argc == 4) && (strcmp(argv[3], "-v") == 0) ) {
      verbose = true;
      threshold_msec = atoi(argv[2]);
  } else {
    fprintf (stderr, "FATAL syntax error. %s\n", USAGE);
    exit (1);
  }

  if (threshold_msec < MIN_THRESHOLD_MSEC) {
    fprintf (stderr, "FATAL invalid threshold error. %s\n", USAGE);
    exit (1);
  }

  init_tty(argv[1], 1);

  /* Hook signal handler and start 1 timer (1.5s) */
  if (set_handler (SIGALRM, sig_handler, NULL) == ERR) {
    perror ("Set_handler");
    fprintf (stderr, "FATAL error. Setting signal handler\n");
    exit (2);
  }
  if (arm_timer (ITIMER_REAL , 2, 0, 0) == -1) {
    perror ("arm_timer");
    fprintf (stderr, "FATAL error. Starting timer\n");
    exit (2);
  }

  if (!verbose) {
    printf ("Silent mode\n");
  } else {
    printf ("Verbose mode,  accuracy %d ms,  serial setting %s\n", threshold_msec, argv[1]);
  }

  started = false;
  frame_received = false;
  frame_expected = true;
  prev_precision = ' ';
  eti_index = 0;

  for(;;) {
    read_tty (&oct, 1);
    if (oct == 0x02) started = true;
    if (started) {
      store (oct);
      if (oct == 0x0d) {
        /* End of message */
        frame_expected = false;
        decode_and_synchro();
        eti_index = 0;
        if (arm_timer (ITIMER_REAL , 2, 0, 0) == -1) {
          perror ("main Arm_timer");
        }
        frame_expected = true;
      }
    }
  }
}

