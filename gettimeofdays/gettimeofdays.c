#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "boolean.h"
#include "timeval.h"
#include "sem_util.h"
#include "rusage.h"

static int semid;

#define NTIMES 21

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
  timeout_t times[NTIMES];
  int i;
  char buffer[500];
  t_result r;

  if (init_rusage() != RUSAGE_OK) {
    perror("init_rusage");
    exit(1);
  }

  /* Create sem */
  r = create_sem_key (21, &semid);
  if (r == ERR) {
    perror ("create_sem_key");
    exit (1);
  }

  /* Do something */
  usleep (1000000);
  for (i = 1; i <= 100000; i++) {
    if (get_vtime(&t1) != 0) {
      perror("get_vtime");
      exit(1);
    }
  }

  /* Get current time */
  dump_rusage_str ("Start iterations between changes");
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

  /* Mesure time for decr_sem, gettimeofday, incr_sem */
  dump_rusage_str ("Start 1000000 cycles decr_sem&gettimeofday&incr_sem");
  for (i = 1; i <= 1000000; i++) {
    if (get_vtime(&t1) != 0) {
      perror("get_vtime");
      exit(1);
    }
  }
  dump_rusage_str ("After 1000000 cycles decr_sem&gettimeofday&incr_sem");

  /* Mesure time for gettimeofday */
  dump_rusage_str ("Start 1000000 cycles gettimeofday");
  for (i = 1; i <= 1000000; i++) {
    get_time (&t3);
  }
  dump_rusage_str ("After 1000000 cycles gettimeofday");

  /* Mesure time of usleep 1, 10, 100, 1000, 100000, 1000000 */
  dump_rusage_str ("Start usleep 1, 10, 100, 1000, 10000, 100000, 1000000");
  usleep (1);
  dump_rusage_str ("After usleep 1");
  usleep (10);
  dump_rusage_str ("After usleep 10");
  usleep (100);
  dump_rusage_str ("After usleep 100");
  usleep (1000);
  dump_rusage_str ("After usleep 1000");
  usleep (10000);
  dump_rusage_str ("After usleep 10000");
  usleep (100000);
  dump_rusage_str ("After usleep 100000");
  usleep (1000000);
  dump_rusage_str ("After usleep 1000000");

  /* Several successive gettimeofday */
  for (i = 0; i < NTIMES; i++) {
    get_time(&times[i]);
  }
  for (i = 0; i < NTIMES; i++) {
    sprintf(buffer, "Gettime[%04d] -> %06ld.%06ld\n", i, times[i].tv_sec, times[i].tv_usec);
    dump_rusage_str (buffer);
  }

  /* Delete sem & exit */
  delete_sem_id (semid);
  return 0;
}

