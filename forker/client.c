#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>


#include "get_line.h"

#include "socket.h"
#include "forker_messages.h"


/* To is:   "bla\0bla\0...bla\0\0 */
/* becomes: "bla\0bla\0...bla\0str\0\0 */
static void cat_str (char *to, char *str) {
  char p;

  p = ' ';
  while ( (*to != '\0') || (p != '\0') ) {
    p = *to;
    to++;
  }
  strcpy (to, str);
  while (*to != '\0') to++;
  *(to+1) = '\0';
}


int main(int argc, char *argv[]) {

  command_number number;
  boolean udp_mode;
  soc_token soc = NULL;
  int port_no;
  char buff[500];
  request_message_t request;
  report_message_t report;
  int i, n;
  soc_host  my_host;
  soc_port  my_port;


  /* printf("Req_size: %d\n", sizeof(request_message_t)); */
  /* printf("Rep_size: %d\n", sizeof(report_message_t)); */

  udp_mode = -1;
  if (argc == 4) {
    if (strcmp (argv[1], "-u") == 0) {
      udp_mode = TRUE;
    } else if (strcmp (argv[1], "-t") == 0) {
      udp_mode = FALSE;
    }
  }
  if (udp_mode == -1) {
    fprintf(stderr, "Error. Two args -u/-t <hostname> <port_name/num> expected.\n");
    exit(1);
  }

  /* Socket stuff */
  if (soc_open(&soc, (udp_mode ? udp_socket : tcp_header_socket)) != SOC_OK) {
    perror("soc_open");
    exit(1);
  }
  if (udp_mode && soc_link_dynamic(soc) != SOC_OK) {
    perror("soc_link_dynamic");
    exit(1);
  }
  port_no = atoi(argv[3]);
  if (port_no <= 0) {
    if (soc_set_dest_name_service(soc, argv[2], FALSE, argv[3]) != SOC_OK) {
      perror("soc_set_dest_service");
      exit (1);
    }
  } else {
    if (soc_set_dest_name_port(soc, argv[2], FALSE, port_no) != SOC_OK) {
      perror("soc_set_dest_port");
      exit (1);
    }
  }
  if (soc_set_blocking(soc, FALSE) != SOC_OK) {
    perror("soc_set_blocking");
    exit(1);
  }

  /* Get current host and port */
  if (soc_get_local_host_id(&my_host) != SOC_OK) {
    perror ("soc_get_local_host");
    exit(1);
  }
  my_port = 0;
  if (udp_mode && soc_get_linked_port(soc, &my_port) != SOC_OK) {
    perror("soc_get_linked_port");
    exit(1);
  }
  printf ("I am host %u port %u\n", my_host.integer, my_port);

  number = 0;
  for (;;) {
    printf ("\n");

    printf ("Start Kill Exit Ping Read (s k e p r) ? ");
    i = get_line (NULL, buff, sizeof(buff));

    if (strcmp(buff, "s") == 0) {
      /* Start */
      memset(request.start_req.command_text, 0,
             sizeof(request.start_req.command_text));
      memset(request.start_req.environ_variables, 0,
             sizeof(request.start_req.environ_variables));
      request.kind = start_command;
      printf ("Number: %d\n", number);
      request.start_req.number = number;
      number += 1;
      printf ("Command ? ");
      i = get_line (NULL, request.start_req.command_text,
                  sizeof(request.start_req.command_text));
      for (;;) {
        printf ("Argument (Empty to end) ? ");
        i = get_line (NULL, buff, sizeof(buff));
        if (i == 0) break;
        cat_str (request.start_req.command_text, buff);
      }
      for (n = 1;;n++) {
        printf ("Environ (Empty to end) ? ");
        i = get_line (NULL, buff, sizeof(buff));
        if (i == 0) break;
        if (n == 1) {
          strcpy (request.start_req.environ_variables, buff);
        } else {
          cat_str (request.start_req.environ_variables, buff);
        }
      }

      printf ("Current dir ? ");
      i = get_line (NULL, request.start_req.currend_dir,
                    sizeof(request.start_req.currend_dir));

      printf ("Output flow ? ");
      i = get_line (NULL, request.start_req.output_flow,
                    sizeof(request.start_req.output_flow));
      if (i != 0) {
        for (;;) {
          printf ("  Append (t or [f]) ? ");
          i = get_line (NULL, buff, sizeof(buff));
          if (strcmp(buff, "t") == 0) {
            request.start_req.append_output = 1;
            break;
          } else if ( (i == 0) || (strcmp(buff, "f") == 0) ) {
            request.start_req.append_output = 0;
            break;
          }
        }
      }

      printf ("Error flow ? ");
      i = get_line (NULL, request.start_req.error_flow,
                    sizeof(request.start_req.error_flow));
      if (i != 0) {
        for (;;) {
          printf ("  Append (t or [f]) ? ");
          i = get_line (NULL, buff, sizeof(buff));
          if (strcmp(buff, "t") == 0) {
            request.start_req.append_error = 1;
            break;
          } else if ( (i == 0) || (strcmp(buff, "f") == 0) ) {
            request.start_req.append_error = 0;
            break;
          }
        }
      }

      if (soc_send(soc, (char*)&request, sizeof(request)) != SOC_OK) {
        perror("soc_send");
      }

    } else if (strcmp(buff, "k") == 0) {
      /* Kill */
      request.kind = kill_command;
      for (;;) {
        printf ("Number ? ");
        i = get_line (NULL, buff, sizeof(buff));
        i = atoi(buff);
        if (i >= 0) break;
      }
      request.start_req.number = i;
      for (;;) {
        printf ("Signal ? ");
        i = get_line (NULL, buff, sizeof(buff));
        i = atoi(buff);
        if (i >= 0) break;
      }
      request.kill_req.signal_number = i;

      if (soc_send(soc, (char*)&request, sizeof(request)) != SOC_OK) {
        perror("soc_send");
      }

    } else if (strcmp(buff, "e") == 0) {
      /* Exit */
      request.kind = fexit_command;
      for (;;) {
        printf ("Exit code ? ");
        i = get_line (NULL, buff, sizeof(buff));
        i = atoi(buff);
        if (i >= 0) break;
      }
      request.fexit_req.exit_code = i;

      if (soc_send(soc, (char*)&request, sizeof(request)) != SOC_OK) {
        perror("soc_send");
      }

    } else if (strcmp(buff, "p") == 0) {
      /* Exit */
      request.kind = ping_command;

      if (soc_send(soc, (char*)&request, sizeof(request)) != SOC_OK) {
        perror("soc_send");
      }

    } else if (strcmp(buff, "r") == 0) {
      /* Read report */
      n = soc_receive(soc, &report, sizeof(report), FALSE);
      if (n == sizeof(report)) {
        switch (report.kind) {
          case start_report :
            printf ("Start: command %d pid %d\n", report.start_rep.number,
                    report.start_rep.started_pid);
          break;
          case kill_report :
            printf ("Kill: command %d pid %d\n", report.kill_rep.number,
                    report.kill_rep.killed_pid);
          break;
          case exit_report :
            printf ("Exit: command %d pid %d code %d ", report.exit_rep.number,
                    report.exit_rep.exit_pid, report.exit_rep.exit_status);
            if (WIFEXITED(report.exit_rep.exit_status)) {
              printf ("exit normally code %d\n",
                      WEXITSTATUS(report.exit_rep.exit_status));
            } else if (WIFSIGNALED(report.exit_rep.exit_status)) {
              printf ("exit on signal %d\n",
                      WTERMSIG(report.exit_rep.exit_status));
            } else {
              printf ("stopped on signal %d\n",
                      WSTOPSIG(report.exit_rep.exit_status));
            }
          break;
          case fexit_report :
            printf ("Forker exited\n");
          break;
          case pong_report :
            printf ("Pong\n");
          break;
          default :
           printf ("Unknown report kind\n");
          break;
        }
      } else if (n == SOC_WOULD_BLOCK) {
        printf ("No report\n");
      } else if (n == SOC_READ_0) {
        printf ("Disconnected\n");
        exit (0);
      } else {
        perror("soc_receive");
      }
    }
  }
}

