#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <libgen.h>

char prog_name[500];


static void usage(const char *msg) {
  fprintf (stderr, "%s\n", msg);
  fprintf (stderr, "Usage : %s <seconds>   |   [[dd/mm/yyyy] hh:mm:ss]\n", prog_name);
  fprintf (stderr, "Examples : %s 81365\n           %s 25/10/1971 10:30:25\n           %s 12:00:01\n",
           prog_name, prog_name, prog_name);
}

static int to_int(char *str) {
  int i, r;

  r = atoi(str);
  if (r == 0) {
    for (i = 0; i < strlen(str) ; i ++) {
      if (str[i] != '0') return -1;
    }
  }
  return (r);
}

static void printd (int sec) {
  struct tm *tm_date_p;
  /* String of the date printed : year month day hours:min:sec */
  char printed_date [133];

  tm_date_p = gmtime( (time_t*) &(sec) );
  if (tm_date_p == (struct tm*)NULL) {
    perror("gmtime");
    fprintf (stderr, "Cannot convert %d seconds in date. Abort.\n", sec);
    exit (1);
  }

  /* dd/mm/yyyy hh:mm:ss */
  sprintf (printed_date, "%02d/%02d/%04d %02d:%02d:%02d",
      tm_date_p->tm_mday, (tm_date_p->tm_mon)+1, (tm_date_p->tm_year)+1900,
      tm_date_p->tm_hour, tm_date_p->tm_min, tm_date_p->tm_sec);

  printf ("%d secs -> %s GMT\n", sec, printed_date);
}

int main(int argc, char *argv[]) {

  time_t timeoffset;
  struct timeval time;

  /* Struct returned by gmtime */
  struct tm tm_date, *tm_date_p;
  char buff[50];
  int digits;
  int i, j, k;

  strcpy (prog_name, basename(argv[0]));

  if ((argc == 2)  && (strcmp(argv[1], "-h") == 0) ) {
    usage("");
    exit(0);
  }

  if (argc == 1) {
    digits = 0;
  } else if (argc == 2) {
    /* One arg : either secs (digits) or hh:mm:ss */
    digits = 1;
    for (i = 0; i < strlen(argv[1]); i++) {
      if (!isdigit(argv[1][i])) {
        /* Not a digit : so this might be hh:mm:ss (check later on) */
        digits = 0;
        break;
      }
    }
  } else if (argc == 3) {
    digits = 0;
  } else {
    usage("SYNTAX ERROR");
    exit(1);
  }

  /* Compute local time offset */
  if (gettimeofday(&time, (struct timezone *)NULL) == -1) {
    perror ("gettimeofday");
    fprintf (stderr, "Cannot get current date and time. Abort.\n");
    exit (2);
  }
  tm_date_p = gmtime ((time_t*) &time.tv_sec);
  if (tm_date_p == (struct tm*)NULL) {
    perror("gmtime");
    fprintf (stderr, "Cannot convert current date. Abort.\n");
    exit (1);
  }

  timeoffset = mktime (tm_date_p);
  if (timeoffset == -1) {
    perror("mktime");
    fprintf (stderr, "Cannot convert date 0 to localtime. Abort.\n");
    exit (1);
  }
  timeoffset = time.tv_sec - timeoffset;
  printf ("Local time offset is %ds\n", (int)timeoffset);

  if (digits == 1) {
    /* One arg: tv_secs */
    /* Seconds from 01/01/1979 00:00:00  -->   dd/mm/yyyy hh:mm:ss */
    time.tv_sec = to_int(argv[1]);
    if (time.tv_sec == -1) {
      fprintf (stderr, "Cannot convert >%s< string in int. Abort.\n", argv[1]);
      usage("SYNTAX ERROR");
      exit (1);
    }

    printd (time.tv_sec);
    /* Done */
    exit(0);
  }

  if (argc == 1) {
    /* No arg -> gettimeofday */
    if (gettimeofday(&time, (struct timezone *)NULL) == -1) {
      perror ("gettimeofday");
      fprintf (stderr, "Cannot get current date and time. Abort.\n");
      exit (2);
    }

    printd (time.tv_sec);
    /* Done */
    exit(0);
  }

  /* [ dd/mm/yyyy ] hh:mm:ss */
  /* Check length */
  if ( ( (argc == 2) && (strlen(argv[1]) != 8) )
    || (  (argc == 3) && (strlen(argv[1]) != 10) && (strlen(argv[2]) != 8) ) ) {

    usage("SYNTAX ERROR");
    exit(1);
  }

  /* Check digits of dd/mm/yyyy if any, then check hh:mm:ss */
  if (argc == 3) {
    strcpy (buff, argv[2]);
    if ( (!isdigit(argv[1][0])) || (!isdigit(argv[1][1])) || (argv[1][2] != '/')
      || (!isdigit(argv[1][3])) || (!isdigit(argv[1][4])) || (argv[1][5] != '/')
      || (!isdigit(argv[1][6])) || (!isdigit(argv[1][7]))
      || (!isdigit(argv[1][8])) || (!isdigit(argv[1][9])) ) {
      usage("SYNTAX ERROR");
      exit(1);
    }
  } else {
    strcpy (buff, argv[1]);
  } 

  if ( (!isdigit(buff[0])) || (!isdigit(buff[1])) || (buff[2] != ':')
    || (!isdigit(buff[3])) || (!isdigit(buff[4])) || (buff[5] != ':')
    || (!isdigit(buff[6])) || (!isdigit(buff[7])) ) {
    usage("SYNTAX ERROR");
    exit(1);
  }

  /* Set date */
  bzero ((char*)&tm_date, sizeof(struct tm));

  if (argc == 2) {
    /* No year month day : get current */
    if (gettimeofday(&time, (struct timezone *)NULL) == -1) {
      perror ("gettimeofday");
      fprintf (stderr, "Cannot get current date and time. Abort.\n");
      exit (2);
    }

    tm_date_p = localtime( (time_t*) &(time.tv_sec) );
    if (tm_date_p == (struct tm*)NULL) {
      perror("localtime");
      fprintf (stderr, "Cannot convert current date. Abort.\n");
      exit (1);
    }
    bcopy ((char*)tm_date_p, (char*)&tm_date, sizeof(struct tm));
  } else {
    /* Load tm structure (year month day) from argv[1] */
    strcpy(buff, argv[1]);
    buff[2] = '\0';
    buff[5] = '\0';
    i = to_int(&buff[0]);
    j = to_int(&buff[3]);
    k = to_int(&buff[6]);
    if ( (i == -1) || (j == -1) || (k == -1) || (i < 0) || (i > 31) || (j < 1) || (j > 12) || (k < 1970) ) {
      fprintf (stderr, "Cannot convert >%s< date in date. Abort.\n", argv[1]);
      usage("SYNTAX ERROR");
      exit (1);
    }
    tm_date.tm_mday = i;
    tm_date.tm_mon = j - 1;
    tm_date.tm_year = k - 1900;
  }

  /* Load tm structure (hour min sec) from argv */
  if (argc == 2) {
    strcpy (buff, argv[1]);
  } else {
    strcpy (buff, argv[2]);
  }

  buff[2] = '\0';
  buff[5] = '\0';
  i = to_int(&buff[0]);
  j = to_int(&buff[3]);
  k = to_int(&buff[6]);
  if ( (i == -1) || (j == -1) || (k == -1) || (i > 23) || (j > 59) || (k > 59) ) {
      fprintf (stderr, "Cannot convert >%s< time in date. Abort.\n", buff);
      usage("SYNTAX ERROR");
      exit (1);
  }
  tm_date.tm_hour = i;
  tm_date.tm_min = j;
  tm_date.tm_sec = k;

  /* Compute secs since 01/10/1970 00:00:00 localtime */
  tm_date.tm_isdst = 0;
  time.tv_sec = mktime(&tm_date) + timeoffset;

  if (time.tv_sec == (time_t)-1) {
    perror("mktime");
    if (argc == 2) {
      fprintf (stderr, "Cannot convert >%s< to time. Abort.\n", argv[1]);
    } else {
      fprintf (stderr, "Cannot convert >%s< >%s< to date. Abort.\n", argv[1], argv[2]);
    }
    exit (1);
  }

  printd (time.tv_sec);

  exit (0);

}

