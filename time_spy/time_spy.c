#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "socket.h"
#include "timeval.h"

#define MSG_LEN 30
static char msg[MSG_LEN];


static char week_day[7][4] = {"DIM", "LUN", "MAR", "MER", "JEU", "VEN", "SAM"};

static void print_time (timeout_t *p_time) {
  char *date_str;
  char buf[50];
  struct tm *date_struct;

  date_str = ctime ( (time_t*) &(p_time->tv_sec));
  date_struct = gmtime ( (time_t*) &(p_time->tv_sec));


  /* msg[0..2] <- day_name */
  msg[ 0] = week_day[date_struct->tm_wday][0];
  msg[ 1] = week_day[date_struct->tm_wday][1];
  msg[ 2] = week_day[date_struct->tm_wday][2];
 
  msg[ 3] = ' ';

  /* msg[4..5] <- jj */
  sprintf (buf, "%02d", date_struct->tm_mday);
  msg[ 4] = buf[0];
  msg[ 5] = buf[1];
  msg[ 6] = '/';
  /* msg[7..8] <- mm */
  sprintf (buf, "%02d", date_struct->tm_mon+1);
  msg[ 7] = buf[0];
  msg[ 8] = buf[1];
  msg[ 9] = '/';
  /* msg[10..11] <- yy */
  msg[10] = date_str[22];
  msg[11] = date_str[23];

  msg[12] = ' ';
  msg[13] = ' ';

  /* msg[14..21] <- hh:mm:ss */
  strncpy (&(msg[14]), &(date_str[11]), 8);
  
  /* msg 22..28 <- .millisecs */
  msg[22] = '.';
  sprintf (&msg[23], "%06d", (int)p_time->tv_usec);

  msg[29] = '\0';
  
  printf ("%s\n", msg);
  fflush (stdout);
  
}

static void usage (void) {
  printf ("Usage : time_spy\n");
  printf ("or      time_spy -C <udp_port>\n");
  printf ("or      time_spy -S <udp_port> <client_node>\n");
}

static void single (void) {
  timeout_t cur_time, exp_time, delta_time;
  int cr;

  get_time (&cur_time);
  
  exp_time.tv_sec = cur_time.tv_sec + 1;
  exp_time.tv_usec = 0;

  
  for (;;) {


    delta_time.tv_sec = exp_time.tv_sec;
    delta_time.tv_usec = exp_time.tv_usec;
    get_time (&cur_time);
    print_time (&cur_time);
    if (sub_time (&delta_time, &cur_time) > 0) {
      cr = select (0, NULL, NULL, NULL, &delta_time);
    } else {
      cr = 0;
    }
    if (cr < 0) {
      perror ("select");
    }
    exp_time.tv_sec = exp_time.tv_sec + 1;
  }
}

static void client (char *port_def) {
  soc_token soc;
  soc_port port_no;
  char message[500];
  soc_length length;
  timeout_t cur_time;

  if (soc_open(&soc, udp_socket) != SOC_OK) {
    perror ("opening socket\n");
    exit (1);
  }

  port_no = atoi(port_def);
  if (port_no <= 0) {
    if (soc_link_service(soc, port_def) != SOC_OK) {
      perror ("linking socket\n");
      exit (1);
    }
  } else {
    if (soc_link_port(soc, port_no) != SOC_OK) {
      perror ("linking socket\n");
      exit (1);
    }
  }

  printf ("Client ready.\n");

  for (;;) {
    length = sizeof(message);
    if (soc_receive(soc, message, length, TRUE) <= 0) {
      perror ("receiving from socket\n");
    } 
    get_time (&cur_time);
    print_time (&cur_time);
  }

}

static void server (char *port_def, char *server_node) {
  soc_token soc;
  soc_port port_no;
  char message[500];
  soc_length length;
  timeout_t cur_time, exp_time, delta_time;
  int cr;

  if (soc_open(&soc, udp_socket) != SOC_OK) {
    perror ("opening socket\n");
    exit (1);
  }

  port_no = atoi(port_def);
  if (port_no <= 0) {
    if (soc_set_dest_name_service(soc, server_node, false, port_def) != SOC_OK) {
      perror ("setting destination socket\n");
      exit (1);
    }
  } else {
    if (soc_set_dest_name_port(soc, server_node, false, port_no) != SOC_OK) {
      perror ("setting destination socket\n");
      exit (1);
    }
  }

  printf ("Server ready.\n");
  strcpy (message, "Hello.");
  length = strlen(message) +1;


  get_time (&cur_time);
  
  exp_time.tv_sec = cur_time.tv_sec + 1;
  exp_time.tv_usec = 0;

  
  for (;;) {


    delta_time.tv_sec = exp_time.tv_sec;
    delta_time.tv_usec = exp_time.tv_usec;
    get_time (&cur_time);
    if (sub_time (&delta_time, &cur_time) > 0) {
      cr = select (0, NULL, NULL, NULL, &delta_time);
    } else {
      cr = 0;
    }
    if (cr < 0) {
      perror ("select");
    }
    exp_time.tv_sec = exp_time.tv_sec + 1;

    if (soc_send(soc, message, length) != SOC_OK) {
      perror ("sending to socket\n");
    } 
  }


}


int main (int argc, char *argv[]) {
 
  if (argc == 1) {
    single();
  } else if ((argc == 3) && (strcmp(argv[1], "-C") == 0) ) {
    client(argv[2]);
  } else if ((argc == 4) && (strcmp(argv[1], "-S") == 0) ) {
    server (argv[2], argv[3]);
  } else {
    usage();
  }
  exit(0);
}

