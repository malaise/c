#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <ctype.h>

#define LF (char)0x0A
#define TAB (char)0x09

#if defined(alpha)
#define TCPDUMP "/usr/sbin/tcpdump"
#define PFCONFIG "/usr/sbin/pfconfig"
#elif defined(linux)
#define TCPDUMP "/usr/sbin/tcpdump"
#define PFCONFIG ""
#elif defined(cetaix)
#define TCPDUMP "/usr/sbin/tcpdump"
#define PFCONFIG ""
#endif



static char get_char (void) {
  int i;

  i = getchar();
  if (i == EOF) {
    exit (0);
  }
  return (char) i;
}

static int get_day (void) {
  struct timeval time;
  struct tm *pdate;

  gettimeofday(&time, NULL);
  pdate = localtime (&time.tv_sec);
  return (pdate->tm_mday);
}

int main (int argc, char *argv[]) {

  int i, d;
  char c, c1, c2;
  char buffer[512];
  char interface_name[50];
  int size_set;
  int hexa_only;
  int skip_arg;
  int day, prev_hour, hour;
  char pfconfig_cmd_down[50], pfconfig_cmd_up[50], tcpdump_cmd[1024], tcpdump_arg[1024];


  if ( (argc != 2) || (strcmp(argv[1], "-a") != 0) ) {
    /* First tcpdump : config interface */
    /* and start "/usr/sbin/tcpdump | tcpdump -a" */


    interface_name[0] = '\0';
    /* Look for -i (to store interface for pfconfig)     */
    /* -s : otherwise set default size                   */
    /* -h : no | tcpdump -a (not transmitted to tcpdump) */
    size_set = 0;
    hexa_only = 0;
    tcpdump_arg[0] = '\0';
    for (i = 1; i < argc; i++) {
      skip_arg = 0;
      if ( (argv[i][0] == '-') && (argv[i][1] == 'i') ) {
        if (argv[i][2] == '\0') {
          if (i != argc) {
            strcpy (interface_name, argv[i+1]);
          }
        } else {
          strcpy (interface_name, &(argv[i][2]));
        }
      } else if ( (argv[i][0] == '-') && (argv[i][1] == 's') ) {
        size_set = 1;
      } else if ( (argv[i][0] == '-') && (argv[i][1] == 'h') ) {
        hexa_only = 1;
        skip_arg = 1;
      }
      if (skip_arg == 0) {
        strcat (tcpdump_arg, " ");
        strcat (tcpdump_arg, argv[i]);
      }
    }

    if (interface_name[0] == '\0') {
      strcpy (interface_name, "tu0");
    }
    
    strcpy (pfconfig_cmd_up, PFCONFIG);
    strcpy (pfconfig_cmd_down, pfconfig_cmd_up);
    strcat (pfconfig_cmd_up, " +p +c ");
    strcat (pfconfig_cmd_down, " -p -c ");
    strcat (pfconfig_cmd_up, interface_name);
    strcat (pfconfig_cmd_down, interface_name);

    strcpy (tcpdump_cmd, TCPDUMP);
    if (size_set == 0) {
      strcat (tcpdump_cmd, " -s 1514");
    }
    strcat (tcpdump_cmd, tcpdump_arg);

    if (hexa_only == 0) {
      /* /usr/sbin/tcpdump | myself -a */
      strcat (tcpdump_cmd, " | ");
      strcat (tcpdump_cmd, argv[0]);
      strcat (tcpdump_cmd, " -a");
    }

    /* Do it */
    if (PFCONFIG != "") {
      fprintf (stderr, "%s\n", pfconfig_cmd_up);
      system (pfconfig_cmd_up);
    }

    fprintf (stderr, "%s\n", tcpdump_cmd);
    fflush(stderr);
    system (tcpdump_cmd);

    if (PFCONFIG != "") {
      fprintf (stderr, "%s\n", pfconfig_cmd_down);
      system (pfconfig_cmd_down);
    }

  } else {
    /* Myself -a : take standard tcpdump traces as input and */
    /* Add ascii dump of messages */

    /* Ignore signal INT */
    signal (SIGINT, SIG_IGN);

    /* Current day */
    day = get_day();
    prev_hour = -1;

    /* Each line */
    for (;;) {
      i = 0;
      c = get_char();
      if ( (c == ' ') || (c == TAB) ) {
        /* First char is tab, skip all tab, print 5 spaces */
        do {
          c = get_char();
        } while ( (c == ' ') || (c == TAB) );
        (void) printf ("     ");
        /* Dump and store the exa */
        do {
          buffer[i] = c;
          i++;
          (void) putchar(c);
          c = get_char();
        } while (c != LF);
        /* Dump the ascii corresponding ascii */
        buffer[i] = '\0';
        /* Pad end of exa line with spaces */
        while (i < 44) {
          (void) putchar(' ');
          i++;
        }
        (void) printf ("->     ");
        i = 0;
        for (;;) {
          c1 = buffer[i];
          i++;
          c2 = buffer[i];
          i++;
          if (isdigit(c1)) {
            d = (int) (c1 - '0');
          } else {
            d = (int) (c1 - 'a') + 10;
          }
          d = d * 16;
          if (isdigit(c2)) {
            d = d + (int) (c2 - '0');
          } else {
            d = d + (int) (c2 - 'a') + 10;
          }
          c = (char)d;
          if ( (c >= ' ') && (c <= '~') ) {
            (void) putchar ((char)d);
          } else {
            (void) putchar ('.');
          }
          /* Here, there is a space or '\0' */
          if (buffer[i] == '\0') {
            /* End of bufferd line */
            break;
          }
          if (buffer[i] == ' ') {
            /* Skip space */
            i++;
          }
        }

        (void) putchar(LF);       
      } else {
        /* Line does not start with space, copy it */
        i = 0;
        while (c != LF) {
          buffer[i] = c;
          c = get_char();
          i++;
        }
        buffer[i] = '\0';

        sscanf (buffer, "%2d", &hour);
        if ( (prev_hour >= 18) && (hour <= 6) ) {
          day = get_day();
        }
        prev_hour = hour;
        printf ("%02d-%s\n", day, buffer);
      }
    }
    (void) putchar(LF);       

  }
  exit(0);
}

