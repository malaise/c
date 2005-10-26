/****************************************************************/
/*                                                              */
/* Syntax : gorgy <period_sec> <channel>                        */
/* <channel>     ::= <udp_port> | <tty>                         */
/* <udp_port>    ::= -u <udp_port_id>                           */
/* <udp_port_id> ::= <port_name> | <port_number>                */
/* <tty>         ::= -s <tty_conf>                              */
/*                                                              */
/* On udp, gorgy is broadcasting the frame on current lan       */
/*  (implied by current host name) for the port specified as    */
/*  argument.                                                   */
/*                                                              */
/* On tty, tty_conf is <tty_no>:<stopb>:<datab>:<parity>:<bauds>*/
/*  gorgy is sending the frame on the tty named                 */
/*  "/dev/tty<tty_no>" configuring it as follows:               */
/* - input : no setting                                         */
/* - output : ~OPOST (no post processing)                       */
/* - config : IXANY (Xon on any character)                      */
/*            ~(ECHO | ISIG | ICANON | XCASE)                   */
/*              no echo, no special, no canon                   */
/*            VMIN=1, VTIME=0 (emmit imediatly each character)  */
/* <stopb> can be 1 or 2                                        */
/* <datab> can be 7 or 8                                        */
/* <parity> can be ODD, EVEN or OFF                             */
/* <bauds> can be 300, 600, 1200, 1800, 2400, 4800 or 9600      */
/*                                                              */
/* Different frames can be generated depending on compiling     */
/*  defines.                                                    */
/* STANDARD:                                                    */
/* ---------                                                    */
/* The standard frame has the following format (24 characters)  */
/* <STX>NNN DD/MM/YY  hh:mm:ss<CR>                              */
/* where <STX>    is ASCII 2                                    */
/*       NNN      is the day name in 3 uppercase letters        */
/*                (DIM, LUN, MAR, MER, JEU, VEN, SAM)           */
/*       one space                                              */
/*       JJ/MM/YY is the date at format day/month/year, each on */
/*                  2 digits (0 inserted if necessary)          */
/*       two spaces                                             */
/*       hh/mm/ss is the time at format hour/minute/second, each*/
/*                  on 2 digits (0 inserted if necessary)       */
/*       <CR>     is ASCII 13                                   */
/* PALLAS:                                                      */
/*--------                                                      */
/* The Pallas frame  has the following format (25 characters)   */
/* 'T':YY:MM:DD:NNN:hh:mm:ss<CR><LF>                            */
/* where <LF>     is ASCII 10                                   */
/* PALLAS:                                                      */
/*--------                                                      */
/* The Taaats frame  has the following format (25 characters)   */
/* <STX>NNN DD/MM/YY  hh:mm:ssP<CR>                             */
/* where P        is the precision                              */
/*                  '?' 0.5s < error       or incorrect time    */
/*                  '#' 0.1s < error <= 0.5s                    */
/*                  '*' 10ms < error <= 0.1s                    */
/*                  '.'  1ms < error <= 10ms                    */
/*                  ' '        error <= 1ms                     */
/* DACOTA:                                                      */
/* -------                                                      */
/* The Dacota frame  has the following format (15 characters)   */
/* <SOH>YYMMDDhhmmss<EOT>C                                      */
/* where <SOH>    is ASCII 1                                    */
/*       <EOT>    is ASCII 4                                    */
/*       C        is a checksum, xor of bytes from 2nd byte     */
/*                  to <EOT> included                           */
/*                                                              */
/* The frame is sent each <period_sec> second.                  */
/*                                                              */
/****************************************************************/


#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>

#include "timeval.h"

#include "tty.h"
#include "udp.h"
#include "sig_util.h"

#define USAGE() {printf ("Usage : gorgy <period_sec> -s <tty_conf>\n"); \
                 printf ("or      gorgy <period_sec> -u <udp_port>\n"); }

#define NUL (char) 0x00
#define SOH (char) 0x01
#define STX (char) 0x02
#define EOT (char) 0x04
#define LF  (char) 0x0a
#define CR  (char) 0x0d

#ifdef TAAATS
static char precision;
static void sig_handler (int signum) {
  if (signum == 27) {
      precision = ' ';
  } else if (signum == 28) {
      precision = '.';
  } else if (signum == 29) {
      precision = '*';
  } else if (signum == 30) {
      precision = '#';
  } else if (signum == 31) {
      precision = '?';
  }
  printf ("New precision -> '%c'\n", precision);
}
#endif


#define MAX_MSG_LEN 132
static char msg[MAX_MSG_LEN];
static int msg_len = 0;



static char week_day[7][4] = {"DIM", "LUN", "MAR", "MER", "JEU", "VEN", "SAM"};

static void format_time (timeout_t *p_time) {
  char *date_str;
  struct tm *date_struct;

  char yyyy[5], mm[3], dd[3], nnn[4], hh[3], mn[2], ss[2];
  char *yy = &yyyy[2];

  date_str = ctime ( (time_t*) &(p_time->tv_sec));
  date_struct = gmtime ( (time_t*) &(p_time->tv_sec));

  strncpy (yyyy, &date_str[20], 4);
  yyyy[4] = NUL;
  sprintf (mm, "%02d", date_struct->tm_mon+1);
  mm[2] = NUL;
  sprintf (dd, "%02d", date_struct->tm_mday);
  dd[2] = NUL;
  strncpy (nnn, week_day[date_struct->tm_wday], 3);
  nnn[3] = NUL;
  strncpy (hh, &date_str[11], 2);
  hh[2] = NUL;
  strncpy (mn, &date_str[14], 2);
  mn[2] = NUL;
  strncpy (ss, &date_str[17], 2);
  ss[2] = NUL;

#if defined(STANDARD)
  sprintf (msg, "%1c%3s %2s/%2s/%2s  %2s:%2s:%2s%1c", STX, nnn, dd, mm, yy, hh, mn, ss, CR);
  msg_len = 24;
#elif defined(PALLAS)
  sprintf (msg, "%1c:%2s:%2s:%2s:%3s:%2s:%2s:%2s%1c%1c", 'T', yy, mm, dd, nnn, hh, mn, ss, CR, LF);
  msg_len = 25;
#elif defined(TAAATS)
  sprintf (msg, "%1c%3s %2s/%2s/%2s  %2s:%2s:%2s%1c%1c", STX, nnn, dd, mm, yy, hh, mn, ss, precision, CR);
  msg_len = 25;
#elif defined(DACOTA)
  sprintf (msg, "%1c%2s%2s%2s%2s%2s%2s%1c%1c", SOH, yy, mm, dd, hh, mn, ss, EOT, NUL);
  msg_len = 15;
  /* Compute LRC value (xor from 2nd byte to EOT included) */
  {
    int i;
    unsigned char lrc;

    lrc = 0;
    for (i = 1; i <= 13; i++) {
      lrc ^= (unsigned char) msg[i];
    }
    msg[14] = lrc;
  }
#else
define expected: STANDARD PALLAS DACOTA TAAATS
#endif
}

int main (int argc, char *argv[]) {

  int period_sec;
  int cr;
  timeout_t cur_time, exp_time, delta_time;
#ifdef TAAATS
  int pid = getpid();
#endif

  typedef enum {serial, udp} channel_list;
  channel_list channel;


  if ( (argc != 4)
    || ( (strcmp(argv[2], "-u") != 0) && (strcmp(argv[2], "-s") != 0) ) ) {
    USAGE();
    exit(1);
  }

  period_sec = 0;
  period_sec = atoi (argv[1]);
  if ( (period_sec <= 0) || (period_sec >= 3600) ) {
    USAGE();
    exit(1);
  }

  printf ("Period is %d s.\n", (int) period_sec);

  if (strcmp(argv[2], "-u") == 0) {
    channel = udp;
    init_udp(argv[3]);
  } else {
    channel = serial;
    init_tty(argv[3], 0);
  }

#ifdef TAAATS
  (void) set_handler (27, sig_handler, NULL);
  (void) set_handler (28, sig_handler, NULL);
  (void) set_handler (29, sig_handler, NULL);
  (void) set_handler (30, sig_handler, NULL);
  printf ("kill -28  %5d  -> '.'\n", pid);
  printf ("kill -29  %5d  -> '*'\n", pid);
  printf ("kill -30  %5d  -> '#'\n", pid);
  printf ("kill -31  %5d  -> '?'\n", pid);
  precision = '*';
  printf ("Precision -> '%c'\n", precision);
#endif

  get_time (&cur_time);

  exp_time.tv_sec = cur_time.tv_sec + 1;
  exp_time. tv_usec = 0;


  for (;;) {
    format_time (&exp_time);

    delta_time.tv_sec = exp_time.tv_sec;
    delta_time.tv_usec = exp_time.tv_usec;
    get_time (&cur_time);
    if (sub_time (&delta_time, &cur_time) > 0) {
      cr = select (0, NULL, NULL, NULL, &delta_time);
    } else {
      cr = 0;
    }
    if (cr == 0) {
      if (channel == udp) {
        send_udp((char*)msg, msg_len);
      } else {
        send_tty ((char*)msg, msg_len);
      }
      exp_time.tv_sec = exp_time.tv_sec + period_sec;
      /* printf ("Sent.\n"); */
    } else if (errno != EINTR) {
      perror ("select");
      exp_time.tv_sec = exp_time.tv_sec + period_sec;
    }
  }
}

