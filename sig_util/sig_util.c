#include <stdio.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <errno.h>

#include "sig_util.h"

#define MILLION (t_time)1000000




t_result set_handler (int sig_num, void (*sig_handler)(int signum),
                                   void (**old_handler)(int signum) ) {
  void (*loc_old_handler)(int signum);


  loc_old_handler = signal (sig_num, sig_handler);
  if ( ((int)loc_old_handler != -1) && (old_handler != NULL) ) {
    *old_handler = loc_old_handler;
  }

  if ( (int)loc_old_handler != -1) {
    return (OK);
  } else {
    perror ("set_handler.signal");
    return (ERR);
  }
}


/* Normalises a time */
/* returns 1, 0, -1 reflecting the sign of the result */
static int normal (t_time *p_sec, t_time *p_usec) {
  while (*p_usec < 0) {
    (*p_sec) --;
    (*p_usec) += MILLION;
  }

  while (*p_usec >= MILLION) {
    (*p_sec) ++;
    (*p_usec) -= MILLION;
  }

  /* Ok if time is > 0 */
  if ( (*p_sec > 0) || ( (*p_sec == 0) && (*p_usec > 0) ) ) {
    return (1);
  } else if ( (*p_sec == 0)  && (*p_usec == 0) ) {
    return (0);
  } else {
    return (-1);
  }
}


t_result arm_timer (int timer, t_time sec, t_time usec, int repeat) {

  struct itimerval val;

  /* Check and normalise delay, 0 if negative */
  if ( normal(&sec, &usec) <= 0 ) {
    val.it_value.tv_sec  = 0;
    val.it_value.tv_usec = 0;
  } else {
    val.it_value.tv_sec  = sec;
    val.it_value.tv_usec = usec;
  }

  /* Check and normalise 1st time, done if negative */
  normal(&val.it_value.tv_sec, &val.it_value.tv_usec);

  /* set interval; if not repeat, set it to 0 */
  if (repeat > 0) {
    val.it_interval.tv_sec  = sec;
    val.it_interval.tv_usec = usec;
  } else {
    val.it_interval.tv_sec  = 0;
    val.it_interval.tv_usec = 0;
  }


  if (setitimer (timer, &val, NULL) == -1) {
    perror ("arm.setitimer");
    return (ERR);
  } else {
    return (OK);
  }
}



