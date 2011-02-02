/* Analyser of rusage dumps */

#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <strings.h>
#include <stdlib.h>
#include <string.h>

#include "rusage.h"

/* Computes the difference between 2 times : td = t2 - t1 */
#ifdef __STDC__
static void delta (struct timeval *t1, struct timeval *t2, struct timeval *td)
#else
static void delta (t1, t2, td)
  struct timeval *t1, *t2, *td;
#endif
{
  td->tv_sec = t2->tv_sec - t1->tv_sec;
  td->tv_usec = t2->tv_usec - t1->tv_usec;

  if (td->tv_usec < 0) {
    td->tv_usec += 1000000;
    (td->tv_sec)--;
  }
}

/* Computes the sum of 2 times : ts = t1 + t2 */
#ifdef __STDC__
static void sum (struct timeval *t1, struct timeval *t2, struct timeval *ts)
#else
static void sum (t1, t2, ts)
  struct timeval *t1, *t2, *ts;
#endif
{
  ts->tv_sec = t1->tv_sec + t2->tv_sec;
  ts->tv_usec = t1->tv_usec + t2->tv_usec;

  if (ts->tv_usec > 1000000) {
    ts->tv_usec -= 1000000;
    (ts->tv_sec)++;
  }
}

/* Converts a time in a float */
#ifdef __STDC__
static float to_float (struct timeval *time_ptr)
#else
static float to_float (time_ptr)
   struct timeval *time_ptr;
#endif
{
  float f;

  f = (float)time_ptr->tv_sec + (float) time_ptr->tv_usec / 1000000.0;
  return (f);
}

/* Anal_rusage */
#ifdef __STDC__
int main (int argc, char **argv)
#else
int main ( argc, argv)
int argc;
char *argv[];
#endif
{

  /* File and block to read */
  FILE *file;
  size_t n_read;
  file_block block, old_block;

  /* String of the date returned by ctime : day_name month day hours:min:sec year*/
  char *date;
  /* String of the date printed : year month day hours:min:sec */
  char printed_date [133];

  /* Delta with previous record */
  struct timeval delta_time;
  /* Delta with previous record */
  float time_delta = 0.0;
  /* Sum of user and system cpu times */
  struct timeval sum_time;
  float total_cpu;
  float val_delta1 = 0.0, val_delta2 = 0.0, val_percent1, val_percent2;
  int i_delta;

  int i;

  /* Must be launched with file_name as argument */
  if (argc != 2) {
    (void) printf ("Usage : anal_usage file_name\n");
    exit (1);
  }

  /* open file */
  file = fopen (argv[1], "r");
  if (file == (FILE*) NULL) {
    (void) printf ("Error. Can't open file %s.\n", argv[1]);
    exit (1);
  }


  /* read blocks */
  for (i=1; 1; i++) {
    n_read = fread ((void*)&block, sizeof(file_block), 1, file);
    if (n_read != 1) {
        break;
    }

    /* Convert time structure in a string */
    date = ctime (&(block.time.tv_sec) );
    /* Printed_date[0..3] <- year */
    (void) strncpy (&(printed_date[0]), &(date[20]), 4);
    printed_date[4] = ' ';
    /* Printed_date[5..19] <- month  day time */
    (void) strncpy (&(printed_date[5]), &(date[4]), 15);
    printed_date[20] = '\0';

    /* Title of the record, number and time stamp */
    (void) printf ("\nResources usage record no %09d at %s.%06d\n", i, printed_date,
                   (int)block.time.tv_usec);
    if (block.msg[0] != '\0') {
      (void) printf("%s\n", block.msg);
    }
    (void) printf (  "------------------------------------------------------------------\n");
    if ( i != 1 ) {
      /* Time between this record and previous one */
      delta (&old_block.time, &block.time, &delta_time);

      if ((delta_time.tv_sec == 0) && (delta_time.tv_usec == 0)) {
        delta_time.tv_usec = 1;
      }

      time_delta = to_float(&delta_time);
      (void) printf ("Previous record was %9.6f sec ago.\n", time_delta);
    }

    /* Total CPU consumption */
    sum (&block.usage.ru_utime, &block.usage.ru_stime, &sum_time);
    total_cpu = to_float (&sum_time);

    /* Memory sizes */
    if (block.pr_size != -1) {
      (void) printf ("Virtual size            (8K page) %9ld", block.pr_size);
      if ( i != 1 ) {
        (void) printf ("    delta %9ld", (block.pr_size - old_block.pr_size));
      }
      (void) printf ("\n");
    }

    /* @@@ This one is in kb on DG, in pages on SUN an on DEC !!! */
    (void) printf ("Max resident set size   (8K page) %9ld\n", block.usage.ru_maxrss);
    (void) printf ("Integral shared text  size (kb.s) %9ld", block.usage.ru_ixrss);
    (void) printf ("                          average  %10.3f\n", (float) block.usage.ru_ixrss / total_cpu);
    /* @@@ This one is not on DG nor SUN */
#ifdef linux
    (void) printf ("Integral shared mem   size (kb.s) %9ld", block.usage.ru_ixrss);
    (void) printf ("                          average  %10.3f\n", (float) block.usage.ru_ixrss / total_cpu);
#endif
    (void) printf ("Integral        data  size (kb.s) %9ld", block.usage.ru_idrss);
    (void) printf ("                          average  %10.3f\n", (float) block.usage.ru_idrss / total_cpu);
    (void) printf ("Integral        stack size (kb.s) %9ld", block.usage.ru_isrss);
    (void) printf ("                          average  %10.3f\n", (float) block.usage.ru_isrss / total_cpu);

    /* Page faults and swapp */
    (void) printf ("Number of page faults without IO  %9ld", block.usage.ru_minflt);
    if ( i != 1 ) {
      i_delta = block.usage.ru_minflt - old_block.usage.ru_minflt;
      (void) printf ("    delta %9d       average  %11.6f /s", i_delta, (float) i_delta / time_delta);
    }
    (void) printf ("\n");

    (void) printf ("Number of page faults with    IO  %9ld", block.usage.ru_majflt);
    if ( i != 1 ) {
      i_delta = block.usage.ru_majflt - old_block.usage.ru_majflt;
      (void) printf ("    delta %9d       average  %11.6f /s", i_delta, (float) i_delta / time_delta);
    }
    (void) printf ("\n");

    (void) printf ("Number of swaps ................  %9ld", block.usage.ru_nswap);
    if ( i != 1 ) {
      i_delta = block.usage.ru_nswap - old_block.usage.ru_nswap;
      (void) printf ("    delta %9d       average  %11.6f /s", i_delta, (float) i_delta / time_delta);
    }
    (void) printf ("\n");

    /* System inputs / outputs */
    (void) printf ("Number of file system  inputs ..  %9ld", block.usage.ru_inblock);
    if ( i != 1 ) {
      i_delta = block.usage.ru_inblock - old_block.usage.ru_inblock;
      (void) printf ("    delta %9d       average  %11.6f /s", i_delta, (float) i_delta / time_delta);
    }
    (void) printf ("\n");

    (void) printf ("Number of file system outputs ..  %9ld", block.usage.ru_oublock);
    if ( i != 1 ) {
      i_delta = block.usage.ru_oublock - old_block.usage.ru_oublock;
      (void) printf ("    delta %9d       average  %11.6f /s", i_delta, (float) i_delta / time_delta);
    }
    (void) printf ("\n");

    /* Messages sent received */
    (void) printf ("Number of messages sent ........  %9ld", block.usage.ru_msgsnd);
    if ( i != 1 ) {
      i_delta = block.usage.ru_msgsnd - old_block.usage.ru_msgsnd;
      (void) printf ("    delta %9d       average  %11.6f /s", i_delta, (float) i_delta / time_delta);
    }
    (void) printf ("\n");

    (void) printf ("Number of messages received ....  %9ld", block.usage.ru_msgrcv);
    if ( i != 1 ) {
      i_delta = block.usage.ru_msgrcv - old_block.usage.ru_msgrcv;
      (void) printf ("    delta %9d       average  %11.6f /s", i_delta, (float) i_delta / time_delta);
    }
    (void) printf ("\n");

    /* Signal delivered */
    (void) printf ("Number of signals delivered ....  %9ld", block.usage.ru_nsignals);
    if ( i != 1 ) {
      i_delta = block.usage.ru_nsignals - old_block.usage.ru_nsignals;
      (void) printf ("    delta %9d       average  %11.6f /s", i_delta, (float) i_delta / time_delta);
    }
    (void) printf ("\n");

    (void) printf ("Number of voluntary switches ...  %9ld", block.usage.ru_nvcsw);
    if ( i != 1 ) {
      i_delta = block.usage.ru_nvcsw - old_block.usage.ru_nvcsw;
      (void) printf ("    delta %9d       average  %11.6f /s", i_delta, (float) i_delta / time_delta);
    }
    (void) printf ("\n");

    /* Context switches */
    (void) printf ("Number of switches due prio/slice %9ld", block.usage.ru_nivcsw);
    if ( i != 1 ) {
      i_delta = block.usage.ru_nivcsw - old_block.usage.ru_nivcsw;
      (void) printf ("    delta %9d       average  %11.6f /s", i_delta, (float) i_delta / time_delta);
    }
    (void) printf ("\n");

    (void) printf ("User time ................ %9ld.%06d s", block.usage.ru_utime.tv_sec,
                                                            (int)block.usage.ru_utime.tv_usec);
    if ( i != 1 ) {
      val_delta1 = to_float(&block.usage.ru_utime) - to_float(&old_block.usage.ru_utime);
      val_percent1 = val_delta1 / time_delta * 100.0;
      (void) printf ("  delta %11.6f s  percentage %10.6f %%", val_delta1, val_percent1);
    }
    (void) printf ("\n");

    /* System and user time */
    (void) printf ("Sys  time ................ %9ld.%06d s", block.usage.ru_stime.tv_sec,
                                                            (int)block.usage.ru_stime.tv_usec);
    if ( i != 1 ) {
      val_delta2 = to_float(&block.usage.ru_stime) - to_float(&old_block.usage.ru_stime);
      val_percent2 = val_delta2 / time_delta * 100.0;
      (void) printf ("  delta %11.6f s  percentage %10.6f %%",  val_delta2, val_percent2);
    }
    (void) printf ("\n");

    /* Total CPU time */
    (void) printf ("                           ------------------");
    if ( i != 1 ) {
      (void) printf ("        -------------             ------------");
    }
    (void) printf ("\n");

    sum (&block.usage.ru_utime, &block.usage.ru_stime, &sum_time);
    (void) printf ("Total cpu time ........... %9ld.%06d s", sum_time.tv_sec, (int)sum_time.tv_usec);
    if ( i != 1 ) {
      (void) printf ("  delta %11.6f s  percentage %10.6f %%\n",
       val_delta1 + val_delta2, (val_delta1 + val_delta2) / time_delta * 100.0);
    }
    (void) printf ("\n");

    /* Store current data as old, for deltas of next record */
    bcopy ((char *)&block, (char *)&old_block, sizeof(file_block));
  }

  (void) printf ("\nDone.\n");
  fclose (file);
  exit (0);
}

