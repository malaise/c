#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <strings.h>

static void error(void) __attribute__ ((noreturn));
static void error(void) {
    fprintf (stderr, "Usage: settime Dd/Mm/YYyy hh:mm:ss [ms]\n");
    exit (1);
}

int main (int argc, char *argv[]) {
  struct tm tms;
  struct timeval tv;
  char a1[11];
  int l1;
  char a2[9];
  int l2;
  char *p, *pp;

  /* DD/MM/YYYY hh:mm:ss [ms] */
  if ((argc != 3) && (argc != 4) ) error();
  l1 = strlen(argv[1]);
  l2 = strlen(argv[2]);
  if (l1 != 10) error();
  if (l2 != 8)  error();

  strcpy (a1, argv[1]);
  strcpy (a2, argv[2]);

  /* Parse a1 */
  pp = argv[1];
  p = strchr(pp, '/');
  if (p == NULL) error();
  *p = '\0';
  if (strlen(pp) != 2) error();
  tms.tm_mday = atoi(pp);
  if ( (tms.tm_mday < 1) || (tms.tm_mday > 31) ) error();

  pp = p + 1;
  p = strchr(pp, '/');
  if (p == NULL) error();
  *p = '\0';
  if (strlen(pp) != 2) error();
  tms.tm_mon = atoi(pp) - 1;
  if ( (tms.tm_mon < 0) || (tms.tm_mon > 11) ) error();

  pp = p + 1;
  p = strchr(pp, '/');
  if (p != NULL) error();
  if (strlen(pp) != 4) error();
  tms.tm_year = atoi(pp) - 1900;
  if (tms.tm_year < 0) error();



  /* Parse a2 */
  pp = argv[2];
  p = strchr(pp, ':');
  if (p == NULL) error();
  *p = '\0';
  if (strlen(pp) != 2) error();
  tms.tm_hour = atoi(pp); 
  if ( (tms.tm_hour < 0) || (tms.tm_hour > 23) ) error();

  pp = p + 1;
  p = strchr(pp, ':');
  if (p == NULL) error();
  *p = '\0';
  if (strlen(pp) != 2) error();
  tms.tm_min = atoi(pp); 
  if ( (tms.tm_min < 0) || (tms.tm_min > 59) ) error();

  pp = p + 1;
  p = strchr(pp, ':');
  if (p != NULL) error();
  if (strlen(pp) != 2) error();
  tms.tm_sec = atoi(pp); 
  if ( (tms.tm_sec < 0) || (tms.tm_sec > 59) ) error();

  /* Make tv_secs */
  tv.tv_sec = (int)mktime(&tms);
  if (tv.tv_sec == -1) {
    perror("mktime");
    error();
  }

  /* make tv_usecs */
  if (argc == 4) {
    tv.tv_usec = atoi(argv[3]);
  } else {
    tv.tv_usec = 0;
  }
  if ( (tv.tv_usec < 0) || (tv.tv_usec >= 1000) ) error();
  tv.tv_usec = tv.tv_usec * 1000;

  printf ("%ld secs %ld usecs.\n", tv.tv_sec, tv.tv_usec);

  /* Set time */
  if (settimeofday(&tv, NULL) == 0) {
    printf ("Done\n");
    exit(0);
  } else {
    perror("settimeofday");
    error();
  }
}

