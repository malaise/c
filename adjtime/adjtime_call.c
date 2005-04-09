#include <errno.h>
#include <stdio.h>
#include "adjtime_call.h"


extern int adjtime_call (struct timeval *new_delta, struct timeval *old_delta) {
  struct timeval loc_delta;

  /* New delta to set or read only */
  if (new_delta != (struct timeval *)NULL) {
    /* New delta to set */
    loc_delta = *new_delta;
    if ( (loc_delta.tv_sec == 0) && (loc_delta.tv_usec == 0) ) {
        /* (0, 0) does not affect! trick */
        loc_delta.tv_usec = 1;
    }
  } else {
    /* To read value without affecting current delta */
    loc_delta.tv_sec = 0;
    loc_delta.tv_usec = 0;
  }

  if (adjtime (&loc_delta, old_delta) != 0) {
    perror("adjtime");
    return (-1);
  }

  /* Restore adjustement if read only */
  if (new_delta == (struct timeval *)NULL) {
    if (adjtime (old_delta, (struct timeval*)NULL) != 0) {
      perror("adjtime");
      return (-1);
    }
  }
  return (0);

}

