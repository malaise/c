#include "boolean.h"

/* Results of calls */
#define OK 0
#define ERR -1

#define t_time int
#define t_result int


/* To set an signal handler to a signal. */
/*  or to restore default handler */
/* See signal.h for sig_num possible values */
/* SIG_DFL and SIG_IGN can be used for default or ignore handlers */
/* Old_handler may be NULL if of no interest */

t_result set_handler (int sig_num, void (*sig_handler)(int),
 void (**old_handler)(int) );

/* To arm one or disarm of the 3 timers. */
/* See sys/time.h for the list of timers */
/* sec and usec specify the interval (delay between 2 signals) */
/* If the delay is <= 0 then the timer is desabled */
/* if not repeat then only one signal will be generated after delay */
/* if repeat     then a signal will be generated repetitively */

t_result arm_timer (int timer, t_time sec, t_time usec, boolean repeat);

