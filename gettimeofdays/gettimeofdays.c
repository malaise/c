#include <stdlib.h>
#include <stdio.h>

#include "boolean.h"
#include "timeval.h"
#include "sem_util.h"
#include "rusage.h"

static int semid;

static int get_vtime (timeout_t *time) {
  t_result r;

  /* Mesure time for decr_sem, gettimeofday, incr_sem */
  r = decr_sem_id (semid, TRUE);
  if (r == ERR) {
    perror ("decr_sem_id");
    return 1;
  }
  get_time (time);
  r = incr_sem_id (semid, TRUE);
  if (r == ERR) {
    perror ("decr_sem_id");
    return 1;

  }
  return 0;
}

int main (void) {
  timeout_t t1, t2, t3;
  int i;
  char buffer[500];
  t_result r;

  if (init_rusage() != RUSAGE_OK) {
    perror("init_rusage");
    exit(1);
  }

  /* Get current time */
  dump_rusage_str ("Start");
  get_time (&t1);

  /* Wait until it changes */
  for (;;) {
    get_time (&t2);
    if (comp_time(&t1, &t2) != 0) {
      break;
    }
  }

  /* Wait until it changes */
  for (i = 1; ; i++) {
    get_time (&t3);
    if (comp_time(&t3, &t2) != 0) {
      break;
    }
  }

  /* Compute delta */
  (void) sub_time (&t3, &t2);

  /* Display result */
  sprintf (buffer, "Changes after %d iterations", i);
  dump_rusage_str (buffer);

  /* Create sem */
  r = create_sem_key (21, &semid);
  if (r == ERR) {
    perror ("create_sem_key");
    exit (1);
  }
  /* Mesure time for decr_sem, gettimeofday, incr_sem */
  dump_rusage_str ("Start");
  for (i = 1; i <= 1000000; i++) {
    if (get_vtime(&t1) != 0) {
      perror("get_vtime");
      exit(1);
    }
  }
  dump_rusage_str ("After 1000000 decr_sem&gettimeofday&incr_sem");
  /* Delete sem */
  delete_sem_id (semid);

  /* Mesure time for gettimeofday */
  dump_rusage_str ("Start");
  for (i = 1; i <= 1000000; i++) {
    get_time (&t3);
  }
  dump_rusage_str ("After 1000000 gettimeofday");

  exit(0);
}

