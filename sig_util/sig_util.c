#include <stdio.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <errno.h>

#include "sig_util.h"

t_result set_handler (int sig_num, void (*sig_handler)(int),
                                   void (**old_handler)(int)) {
  void (*loc_old_handler)(int);


  loc_old_handler = signal (sig_num, sig_handler);
  if ( (loc_old_handler != SIG_ERR) && (old_handler != NULL) ) {
    *old_handler = loc_old_handler;
  }

  if (loc_old_handler != SIG_ERR) {
    return (OK);
  } else {
    perror ("set_handler.signal");
    return (ERR);
  }
}


t_result arm_timer (int timer, t_time sec, t_time usec, boolean repeat) {
  struct itimerval val;

  val.it_value.tv_sec  = sec;
  val.it_value.tv_usec = usec;

  /* set interval; if not repeat, set it to 0 */
  if (repeat) {
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



