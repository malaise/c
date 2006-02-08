#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>
#include <stdarg.h>


#include "boolean.h"
#include "socket.h"

#include "forker_messages.h"

/* Should be defined in stdlib (not on DU 4.0) */
extern int clearenv (void);

/* For error messages */
#define PROG_NAME "FORKER"

/* Exit code when something is wrong */
#define FATAL_ERROR_EXIT_CODE 1

/* Rights of redirections: -rw-r--r-- 0644 */
#define FILE_RIGHTS  (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)

/* Running command */
typedef struct com_cell {
  command_number  number;
  int             pid;
  soc_token       soc;
  soc_host        client_host;
  soc_port        client_port;
  struct com_cell *next, *prev;
} command_cell;
command_cell *first_com_cell, *last_com_cell;

/* Known clients */
typedef struct cli_cell {
  soc_token       soc;
  int             fd;
  struct cli_cell *next, *prev;
} client_cell;
client_cell *first_cli_cell, *last_cli_cell;

/* Fd to write on when child exited */
static int write_on_me;

/* Debug flag */
#ifdef DEBUG
static int debug=TRUE;
#else
static int debug=FALSE;
#endif
#define FORKER_DEBUG "FORKER_DEBUG"

/* Flush traces */
static void flush (void) {
  (void) fflush (stdout);
  (void) fflush (stderr);
}

static char *date_str (void) {
  struct timeval time;
  struct tm *tm_date_p;
  static char printed_date[50];

  gettimeofday (&time, NULL);
  tm_date_p = gmtime( (time_t*) &(time.tv_sec) );
  sprintf (printed_date, "%04d/%02d/%02d %02d:%02d:%02d.%03ld",
        (tm_date_p->tm_year)+1900, (tm_date_p->tm_mon)+1, tm_date_p->tm_mday,
        tm_date_p->tm_hour, tm_date_p->tm_min, tm_date_p->tm_sec,
        time.tv_usec/1000);

  return printed_date;
}

/* Display error and fataf error message */
static void fatal (const char *msg, const char *extra) {
  fprintf(stderr, "%s %s FATAL: %s%s.\n", date_str(), PROG_NAME, msg, extra);
}
static void error (const char *msg, const char *extra) {
  fprintf(stderr, "%s %s ERROR: %s%s.\n", date_str(), PROG_NAME, msg, extra);
}

/* Extract basename of program name */
static char * progpath = NULL;
static const char * progname (void) {
  char *p;
  int i, len;

  if (progpath == NULL) {
    return "\"Noname\"";
  }
  p = progpath;

  len = strlen(p);

  for (i = 0; i < len; i++) {
    if (progpath[i] == '/') {
      p = &(progpath[i+1]);
    }
  }
  return p;
}

/* Display program usage */
static void usage (void) {
  fprintf(stderr, "Usage: %s  <protocol>", progname());
  fprintf(stderr, " <protocol> ::= <udp> | <tcp> | <ipm>\n");
  fprintf(stderr, " <udp>      ::= -u <port>\n");
  fprintf(stderr, " <tcp>      ::= -t <port>\n");
  fprintf(stderr, " <ipm>      ::= -m <port>@<lan>\n");
  fprintf(stderr, " <port>     ::= <port_num> | <port_nane>\n");
  fprintf(stderr, " <lan>      ::= <lan_num> | <lan_name>\n");
}

/* Display trace message */
static void trace (const char *f,...) {
  va_list args;

  printf ("%s ", date_str());

  va_start(args, f);
  vprintf (f, args);
  va_end(args);

  printf ("\n");

}

/* Trace a socket error */
static void terror (const char *call, const int code) {
  if (code != SOC_OK) {
    fprintf(stderr, "%s returned %s.\n", call, soc_error(code));
  }
}

/* Toggle debug on SIGUSR1 */
static void sigusr1_handler (int signum) {
  if (signum != SIGUSR1) {
    error("Handler called but not on sig usr1", "");
    return;
  }
  debug = !debug;
  trace("Caught sigusr1, debug %s", (debug ? "on" : "off"));
}

/* Write a byte on pipe when a child exits */
static void sigchild_handler (int signum) {

  int res;
  char c = 'C';

  if (signum != SIGCHLD) {
    error("Handler called but not on sig child", "");
    return;
  }

  if (debug) {
    trace("Caught sigchild");
  }

  /* Write on pipe */
  for (;;) {
    res = write(write_on_me, &c, 1);
    if (res > 0) break;
    if ( (res == -1) && (errno != EINTR) ) {
      break;
    }
  }
  if (debug) {
    trace("Pipe written");
  }
}

/* Check there is one terminator ('\0') in string */
static boolean has_1_nul (char *str, int len) {
  int i;
  for (i = 0; i < len; i++) {
    if (str[i] == '\0') {
      return TRUE;
    }
  }
  return FALSE;
}

/* Check there are two successive terminators ("\0\0") in string */
static boolean has_2_nuls (char *str, int len) {
  int i;
  char p;

  p = ' ';
  for (i = 0; i < len; i++) {
    if ( (str[i] == '\0') && (p == '\0') ) {
      return TRUE;
    }
    p = str[i];
  }
  return FALSE;
}

/* Fork and launch a command, return forked pid or -1 */
static int do_start_command (start_request_t *request,
                             soc_token soc,
                             soc_host *client_host,
                             soc_port client_port) {
  int res;
  command_cell *cur_com_cell;
  int fd;
  char *start;
  int nargs;
  char **args;

  /* Check there is a command */
  if (request->command_text[0] == '\0') {
    error("Empty command in start message", "");
    return -1;
  }

  /* Check all strings are propoerly terminated */
  if (! has_2_nuls(request->command_text,
                   sizeof(request->command_text)) ) {
    error("Invalid text in start command", "");
    return -1;
  }
  if (! has_2_nuls(request->environ_variables,
                   sizeof(request->environ_variables)) ) {
    error("Invalid text in start environ", "");
    return -1;
  }
  if (! has_1_nul(request->currend_dir,
                  sizeof(request->currend_dir)) ) {
    error("Invalid text in start curdir", "");
    return -1;
  }
  if (! has_1_nul(request->output_flow,
                  sizeof(request->output_flow)) ) {
    error("Invalid text in start stdout", "");
    return -1;
  }
  if (! has_1_nul(request->error_flow,
                  sizeof(request->error_flow)) ) {
    error("Invalid text in start stderr", "");
    return -1;
  }

  /* Procreate */
  flush();
  res = fork();
  if (res == -1) {
    perror("fork");
    error("Cannot fork", "");
    return -1;
  } else if (res != 0) {
    /***************/
    /* Forker code */
    /***************/
    cur_com_cell = malloc(sizeof(command_cell));
    if (cur_com_cell == NULL) {
      perror("malloc");
      error("Cannot malloc a new command cell", "");
      return -1;
    }
    /* Allocate a cell and store command and pid */
    cur_com_cell->pid = res;
    cur_com_cell->number = request->number;
    cur_com_cell->soc = soc;
    memcpy (&(cur_com_cell->client_host), client_host, sizeof(soc_host));
    cur_com_cell->client_port = client_port;
    cur_com_cell->client_port = client_port;
    cur_com_cell->next = first_com_cell;
    cur_com_cell->prev = NULL;
    first_com_cell = cur_com_cell;
    if (cur_com_cell->next != NULL) {
      (cur_com_cell->next)->prev = cur_com_cell;
    } else {
      last_com_cell = cur_com_cell;
    }
    if (debug) {
      trace ("Forked pid %d", res);
    }
    /* Done */
    return res;
  }

  /**************/
  /* Child code */
  /**************/
  /* Set current directory */
  if (request->currend_dir[0] != '\0') {
    if (chdir(request->currend_dir) != 0) {
      perror("chdir");
      error("Cannot change to current directory: ", request->currend_dir);
      exit(FATAL_ERROR_EXIT_CODE);
    }
    if (debug) {
      trace ("Curdir changed to >%s<", request->currend_dir);
    }
  }

  /* Set environnement variables */
  start = request->environ_variables;
  while (*start != '\0') {
    if(putenv(start) != 0) {
      perror("putenv");
      error("Cannot set environement variable: ", start);
      exit(FATAL_ERROR_EXIT_CODE);
    }
    if (debug) {
      trace ("Envir added: >%s<", start);
    }
    /* Find end of string */
    while (*start != '\0') start++;
    /* Start of next env */
    start++;
  }

  /* Output flow */
  if (request->output_flow[0] != '\0') {
    if (debug) {
      trace ("Setting stdout to >%s< append %d", request->output_flow,
              (int)request->append_output);
    }
    fd = open (request->output_flow,
                O_CREAT | O_WRONLY | (request->append_output ? O_APPEND : O_TRUNC),
                FILE_RIGHTS);
    if (fd == -1) {
      perror("open");
      error("Cannot open output flow: ", request->output_flow);
      exit(FATAL_ERROR_EXIT_CODE);
    }
    flush ();
    if (dup2(fd, fileno(stdout)) == -1) {
      perror("dup2");
      error("Cannot duplicate output flow", "");
      exit(FATAL_ERROR_EXIT_CODE);
    }
    close(fd);
    if (debug) {
      trace ("Stdout set to >%s< append %d", request->output_flow,
              (int)request->append_output);
    }
  }

  /* Error flow */
  if (request->error_flow[0] != '\0') {
    if (debug) {
      trace ("Setting stderr to >%s< append %d", request->error_flow,
              (int)request->append_error);
    }
    fd = open (request->error_flow,
                O_CREAT | O_WRONLY | (request->append_error ? O_APPEND : O_TRUNC),
                FILE_RIGHTS);
    if (fd == -1) {
      perror("open");
      error("Cannot open error flow: ", request->error_flow);
      exit(FATAL_ERROR_EXIT_CODE);
    }
    flush ();
    if (dup2(fd, fileno(stderr)) == -1) {
      perror("dup2");
      error("Cannot duplicate error flow", "");
      exit(FATAL_ERROR_EXIT_CODE);
    }
    close(fd);
    if (debug) {
      trace ("Stderr set to >%s< append %d", request->error_flow,
              (int)request->append_error);
    }
  }

  /* Count program name + arguments */
  nargs = 0;
  start = request->command_text;
  while (*start != '\0') {
    nargs++;
    while (*start != '\0') start++;
    start++;
  }

  /* Need one slot more for NULL */
  nargs++;

  /* Allocate table */
  args = malloc(nargs * sizeof(char*));
  if (args == NULL) {
    perror("malloc");
    error("Cannot malloc array of arguments", "");
    exit(FATAL_ERROR_EXIT_CODE);
  }

  /* Fill table[0] <- program name */
  args[0] = request->command_text;
  start = request->command_text;
  while (*start != '\0') start++;
  start++;
  nargs = 1;

  /* Fill table with arguments */
  while (*start != '\0') {
    args[nargs] = start;
    while (*start != '\0') start++;
    start++;
    nargs++;
  }
  args[nargs] = NULL;

  if (debug) {
    int li = 0;
    trace ("");
    printf ("Command line: >%s<", request->command_text);
    while (args[li] != NULL) {
      printf (" >%s<", args[li]);
      li++;
    }
    printf ("\n\n");
    flush ();
  }

  /* Now the exec */
  if (execv(request->command_text, args) == -1) {
    perror("execv");
    error("Cannot exec command: ", request->command_text);
    exit(FATAL_ERROR_EXIT_CODE);
  }

  /* Never reached */
  exit(FATAL_ERROR_EXIT_CODE);
}


/* Check message kind and size are correct and match */
/* len has to be size of request_u or size of current request_t */
static boolean msg_ok (soc_length len, request_kind_list kind) {
  switch (kind) {
    case start_command:
      if (len == sizeof(start_request_t)) return TRUE;
    break;
    case kill_command:
      if (len == sizeof(kill_request_t)) return TRUE;
    break;
    case fexit_command:
      if (len == sizeof(fexit_request_t)) return TRUE;
    break;
    case ping_command:
      if (len == 0) return TRUE;
    break;
    default:
      error("Received a message with invalid command", "");
      return FALSE;
    break;
  }
  if (len == sizeof(request_u)) {
    return TRUE;
  } else {
    error("Received a message of invalid size", "");
    return FALSE;
  }
}

static boolean send (soc_token dest, soc_message msg,  soc_length len) {
  int res;

  /* Try to send */
  res = soc_send(dest, msg, len);
  if (res == SOC_OK) {
    if (debug) {
      trace ("Report message sent");
    }
    return TRUE;
  } else if (res == SOC_WOULD_BLOCK) {
    if (debug) {
      trace ("Overflow while sending report");
    }
    return TRUE;
  } else if (res != SOC_TAIL_ERR) {
    perror("sending on socket");
    terror("soc_send", res);
    error("Cannot send report message", "");
    return FALSE;
  }

  /* Socket was in overflow */
  res = soc_resend(dest);
  if (res != SOC_OK) {
    perror("sending on socket");
    terror("soc_resend", res);
    error("Cannot resend previous report message", "");
    return FALSE;
  }
  if (debug) {
    trace ("Previous report message resent");
  }

  /* Retry to send */
  res = soc_send(dest, msg, len);
  if (res == SOC_OK) {
    if (debug) {
      trace ("Report message sent");
    }
    return TRUE;
  } else if (res == SOC_WOULD_BLOCK) {
    if (debug) {
      trace ("Overflow while sending report");
    }
    return TRUE;
  } else {
    perror("sending on socket");
    terror("soc_send", res);
    error("Cannot send report message", "");
    return FALSE;
  }
}


int main (int argc, char *argv[]) {

  char *debug_var;
  soc_token service_soc = NULL;
  soc_token client_soc = NULL;
  typedef enum {None, Udp, Tcp, Ipm} proto_list;
  int i, arobase;
  proto_list proto;
  int port_no;
  char *port_name;
  char *lan;
  int nfds;
  fd_set saved_mask, select_mask;
  int service_fd, client_fd;
  int pipe_fd[2];
  int res;
  char c;
  pid_t a_pid;
  command_cell *cur_com_cell;
  client_cell  *cur_cli_cell = NULL;
  report_message_t report;
  request_message_t request_message;
  soc_length        request_len;
  soc_host          request_host;
  soc_port          request_port;


  /* Init debug */
  progpath = argv[0];
  debug_var = getenv(FORKER_DEBUG);
  if ( (debug_var != NULL)
    && (strcmp(debug_var, "Y") == 0) ) {
    debug = TRUE;
  } else if ( (debug_var != NULL)
            && (strcmp(debug_var, "N") == 0) ) {
    debug = FALSE;
  } /* Else unchanged from compilation option */


  /* Check and parse the argument: -u/-t/-m then port [@lan]*/
  proto = None;
  if (argc == 1) {
    fatal("Missing argument for protocole", "");
  } else if (argc != 3) {
    fatal("Invalid arguments", "");
  } else {
    if (strcmp (argv[1], "-u") == 0) {
      proto = Udp;
    } else if (strcmp (argv[1], "-t") == 0) {
      proto = Tcp;
    } else if (strcmp (argv[1], "-m") == 0) {
      proto = Ipm;
    } else {
      fatal("Invalid argument: ", argv[1]);
    }
  }
  if (proto == None) {
    usage();
    exit(FATAL_ERROR_EXIT_CODE);
  }

  /* Set port_name */
  port_name = malloc(strlen(argv[2])+1);
  strcpy (port_name, argv[2]);
  port_no = atoi(port_name);

  /* Decode ipm address: <port>@<lan>:* One unique '@', not first nor last */
  if (proto == Ipm) {
    arobase = 0;
    for (i = 0; i < (signed)strlen(argv[2]); i++) {
      if (port_name[i] == '@') {
        if (arobase != 0) {
          /* Several arobase: error */
          arobase = 0;
          break;
        }
        arobase = i;
      }
    }
    /* Check arobase is found not last */
    if (arobase == (signed)strlen(port_name) - 1) arobase = 0;
    if (arobase == 0) {
      fatal("Invalid ipm address: ", port_name);
      usage();
      exit(FATAL_ERROR_EXIT_CODE);
    }
    lan = &port_name[arobase+1];
    port_name[arobase] = '\0';
  }

  /* Create the socket */
  res = soc_open(&service_soc,
               ((proto == Tcp) ? tcp_header_socket : udp_socket) );
  if (res != SOC_OK) {
    perror("opening socket");
    terror("soc_open", res);
    fatal("Cannot create socket", "");
    exit(FATAL_ERROR_EXIT_CODE);
  }

  /* Set IPM LAN */
  if (proto == Ipm) {
    if (port_no <= 0) {
      res = soc_set_dest_name_service(service_soc, lan, TRUE, port_name);
      if (res != SOC_OK) {
        perror("setting socket dest lan name with port name");
        terror("soc_set_dest_name_service", res);
        fatal("Cannot set ipm lan to: ", argv[2]);
        exit (FATAL_ERROR_EXIT_CODE);
      }
    } else {
      res = soc_set_dest_name_port (service_soc, lan, TRUE, port_no);
      if (res != SOC_OK) {
        perror("setting socket dest lan name with port no");
        terror("soc_set_dest_name_port", res);
        fatal("Cannot set ipm lan to: ", argv[2]);
        exit (FATAL_ERROR_EXIT_CODE);
      }
    }
  }

  /* Bind */
  if (port_no <= 0) {
    res = soc_link_service(service_soc, port_name);
    if (res != SOC_OK) {
      perror("linking socket");
      terror("soc_link_service", res);
      fatal("Cannot bind socket to name: ", port_name);
      exit (FATAL_ERROR_EXIT_CODE);
    }
  } else {
    res = soc_link_port(service_soc, port_no);
    if (res != SOC_OK) {
      perror("linking socket");
      terror("soc_link_port", res);
      fatal("Cannot bind socket to no: ", port_name);
      exit (FATAL_ERROR_EXIT_CODE);
    }
  }

  /* Get socket fd for the select */
  res = soc_get_id(service_soc, &service_fd);
  if (res != SOC_OK) {
    perror("getting service socket fd");
    terror("soc_get_id", res);
    fatal("Cannot get service socket fd", "");
    exit (FATAL_ERROR_EXIT_CODE);
  }

  /* Create pipe and save writting fd for sigchild handler */
  if (pipe(pipe_fd) == -1) {
    perror("pipe");
    fatal("Cannot create pipe", "");
    exit (FATAL_ERROR_EXIT_CODE);
  }
  write_on_me = pipe_fd[1];

  /* Close on exec on pipe fds */
  if (fcntl(pipe_fd[0], F_SETFD, FD_CLOEXEC) < 0) {
    perror("fcntl(FD_CLOEXEC)");
    fatal("Cannot set close on exec on pipe_fd[0]", "");
    exit (FATAL_ERROR_EXIT_CODE);
  }
  if (fcntl(pipe_fd[1], F_SETFD, FD_CLOEXEC) < 0) {
    perror("fcntl(FD_CLOEXEC)");
    fatal("Cannot set close on exec on pipe_fd[1]", "");
    exit (FATAL_ERROR_EXIT_CODE);
  }


  /* Build select mask */
  nfds = 0;
  FD_ZERO (&saved_mask);
  FD_SET(service_fd, &saved_mask);
  if (service_fd > nfds) nfds = service_fd;
  FD_SET(pipe_fd[0], &saved_mask);
  if (pipe_fd[0] > nfds) nfds = pipe_fd[0];

  /* Hook handler for child signal */
  if (signal (SIGCHLD, sigchild_handler) == SIG_ERR) {
    perror("signal");
    fatal("Cannot hook signal child handler", "");
    exit (FATAL_ERROR_EXIT_CODE);
  }

  /* Hook handler for usr1 signal */
  if (signal (SIGUSR1, sigusr1_handler) == SIG_ERR) {
    perror("signal");
    fatal("Cannot hook signal usr1 handler", "");
    exit (FATAL_ERROR_EXIT_CODE);
  }

  /* Init lists of commands and clients*/
  first_com_cell = last_com_cell = NULL;
  first_cli_cell = last_cli_cell = NULL;

  /* Clear environ */
  if (clearenv() != 0) {
    perror("clearenv");
    fatal("Cannot clear environment", "");
    exit (FATAL_ERROR_EXIT_CODE);
  }


  /* Ready */
  fprintf(stderr, "%s %s ready. %s\n", date_str(), PROG_NAME, (debug ? "Debug on." : ""));

  /* Main loop */
  for (;;) {

    /* Select loop while EINTR */
    for (;;) {

      /* Flush traces including signals */
      flush ();

      /* The Select */
      memcpy(&select_mask, &saved_mask, sizeof(fd_set));
      res = select(nfds+1, &select_mask, (fd_set*)NULL, (fd_set*)NULL,
                   (struct timeval*)NULL);
      if (res > 0) break;
      if ( (res == -1) && (errno != EINTR) ) break;
    }
    if (res < 0) {
      perror("select");
      continue;
    }

    /* Where data to read? */
    if (FD_ISSET(pipe_fd[0], &select_mask)) {

      /* Data on pipe, read the byte */
      if (debug) {
        trace ("Data on pipe");
      }
      for (;;) {
        res = read(pipe_fd[0], &c, 1);
        if (res > 0) break;
        if ( (res == -1) && (errno != EINTR) ) break;
      }
      if (res < 0) {
        perror("read");
        continue;
      }
      if (debug) {
        trace ("Data read on pipe");
      }

      /* Look for several exited children */
      report.kind = exit_report;
      for (;;) {
        boolean do_send;

        /* Get a dead pid */
        for (;;) {
          res = waitpid((pid_t)-1, &(report.exit_rep.exit_status), WNOHANG);
          if (res > 0) break;
          if ( (res == -1) && (errno != EINTR) ) break;
        }
        if ((res == 0) || ( (res == -1) && (errno == ECHILD) ) ) {
          /* No more child */
          if (debug) {
            trace ("No more child\n");
          }
          break;
        } else if (res < 0) {
          perror("waitpid");
          continue;
        }

        /* Child pid */
        a_pid = res;
        if (debug) {
          trace ("Pid %d exited code %d", a_pid, report.exit_rep.exit_status);
        }

        /* Look for pid in list */
        cur_com_cell = last_com_cell;
        while ( (cur_com_cell != NULL) && (cur_com_cell->pid != a_pid) ) {
          cur_com_cell = cur_com_cell->prev;
        }
        if (cur_com_cell == NULL) {
          char dbg[50];
          sprintf(dbg, "%d", a_pid);
          error("Cannot find in list pid: ", dbg);
          continue;
        }

        /* Found the command number */
        report.exit_rep.number = cur_com_cell->number;
        report.exit_rep.exit_pid = cur_com_cell->pid;
        if (debug) {
          trace ("Command was %d", report.exit_rep.number);
        }

        /* Set dest to client for report */
        do_send = TRUE;
        if (proto != Tcp) {
          client_soc = service_soc;
          res = soc_set_dest_host_port(client_soc, &(cur_com_cell->client_host),
                                       cur_com_cell->client_port);
          if (res != SOC_OK) {
            error("Cannot set dest to client", "");
            terror("soc_set_dest_host_port", res);
            do_send = FALSE;
          }
        } else {
          client_soc = cur_com_cell->soc;
        }
        if (do_send && debug) {
          trace ("Dest set to host %u port %u",
                  cur_com_cell->client_host.integer,
                  cur_com_cell->client_port);
        }

        /* Free the cell */
        if (cur_com_cell->next != NULL) {
          (cur_com_cell->next)->prev = cur_com_cell->prev;
        } else {
          last_com_cell = cur_com_cell->prev;
        }
        if (cur_com_cell->prev != NULL) {
          (cur_com_cell->prev)->next = cur_com_cell->next;
        } else {
          first_com_cell = cur_com_cell->next;
        }
        free(cur_com_cell);

        /* Send report */
        if (do_send) {
          res = send(client_soc, (soc_message)&report, sizeof(report));
          if (debug && res) {
            trace ("Exit report sent");
          }
        }

      } /* For each dead child */
      continue;
    }

    if ( (proto == Tcp) && FD_ISSET(service_fd, &select_mask)) {
      if (debug) {
        trace ("Accepting new client");
      }
      /* Accept */
      client_soc = init_soc;
      res = soc_accept(service_soc, &client_soc);
      if (res  != SOC_OK) {
        perror("accepting new client");
        terror("soc_accept", res);
        error("Cannot accept new client", "");
        continue;
      }
      res = soc_set_blocking(client_soc, FALSE);
      if (res  != SOC_OK) {
        perror("setting non blocking");
        terror("soc_set_blocking", res);
        error("Cannot set new client socket non blocking", "");
      }
      res  = soc_get_id (client_soc, &client_fd);
      if (res  != SOC_OK) {
        perror("getting new client fd");
        terror("soc_get_id", res);
        error("Cannot get fd of new client", "");
        continue;
      }

      /* Insert client cell */
      cur_cli_cell = malloc(sizeof(client_cell));
      if (cur_cli_cell == NULL) {
        perror("malloc");
        error("Cannot malloc a new client cell", "");
        continue;
      }
      cur_cli_cell->soc = client_soc;
      cur_cli_cell->fd = client_fd;
      cur_cli_cell->next = first_cli_cell;
      cur_cli_cell->prev = NULL;
      first_cli_cell = cur_cli_cell;
      if (cur_cli_cell->next != NULL) {
        (cur_cli_cell->next)->prev = cur_cli_cell;
      } else {
        last_cli_cell = cur_cli_cell;
      }

      /* Add to mask */
      FD_SET(client_fd, &saved_mask);
      if (client_fd > nfds) nfds = client_fd;
      if (debug) {
        trace ("New client accepted with fd %d", client_fd);
      }
      continue;

    }

    /* Get client socket */
    client_soc = init_soc;
    if (proto != Tcp) {
      if (FD_ISSET(service_fd, &select_mask)) {
        client_soc = service_soc;
        client_fd  = service_fd;
      }
    } else {
      /* Look for client */
      cur_cli_cell = first_cli_cell;
      while (cur_cli_cell != NULL) {
        if (FD_ISSET(cur_cli_cell->fd, &select_mask)) {
          client_soc = cur_cli_cell->soc;
          client_fd = cur_cli_cell->fd;
          break;
        }
        cur_cli_cell = cur_cli_cell->next;
      }
    }

    if (client_soc == init_soc) {
      int i;
      error("received data on an unknown fd", "");
      for (i = 0; i <= nfds; i++) {
        if (FD_ISSET(i, &select_mask)) {
          FD_CLR(i, &saved_mask);
          trace("removed from mask fd %d", i);
        }
      }
      continue;
    }


    /* A request */
    if (debug) {
      trace ("Data on socket");
    }

    /* Read request */
    request_len = soc_receive(client_soc, &request_message,
                              sizeof(request_message), (proto != Tcp));

    /* Handle Tcp disconnection */
    if ((proto == Tcp) && ( (request_len == SOC_CONN_LOST)
               || (request_len == SOC_READ_0) ) ) {
      if (debug) {
        trace ("Disconnection of client on fd %d", client_fd);
      }

      FD_CLR(client_fd, &saved_mask);
      soc_close (&client_soc);

      /* Free the cell */
      if (cur_cli_cell->next != NULL) {
        (cur_cli_cell->next)->prev = cur_cli_cell->prev;
      } else {
        last_cli_cell = cur_cli_cell->prev;
      }
      if (cur_cli_cell->prev != NULL) {
        (cur_cli_cell->prev)->next = cur_cli_cell->next;
      } else {
        first_cli_cell = cur_cli_cell->next;
      }
      free(cur_cli_cell);
      continue;
    }

    /* Check len */
    if (request_len < SOC_OK) {
      perror("receiving from socket");
      terror("soc_receive", request_len);
      error("Cannot receive request message", "");
      continue;
    }

    /* Check message */
    if ((unsigned int)request_len < sizeof(request_message.kind)) {
      error("Received a message of invalid size", "");
      continue;
    }
    if (! msg_ok (request_len - sizeof(request_message.kind), request_message.kind) ) {
      continue;
    }


    /* Get client host and port */
    res  = soc_get_dest_host(client_soc, &request_host);
    if (res != SOC_OK) {
      terror("soc_get_dest_host", res);
      error("Cannot get client host", "");
      continue;
    }
    res = soc_get_dest_port(client_soc, &request_port);
    if (res != SOC_OK) {
      terror("soc_get_dest_port", res);
      error("Cannot get client port", "");
      continue;
    }
    if (debug) {
      trace ("Client is host %u port %u",
              request_host.integer,
              request_port);
    }

    if (request_message.kind == kill_command) {

      report.kind = kill_report;
      report.kill_rep.number = request_message.kill_req.number;
      if (debug) {
        trace ("Request kill num %d sig %d",
              request_message.kill_req.number,
              request_message.kill_req.signal_number);
      }

      /* Kill command: Find pid from criteria Num, host, port */
      report.kill_rep.killed_pid = -1;
      cur_com_cell = last_com_cell;
      while (cur_com_cell != NULL) {
        if ( (cur_com_cell->number == request_message.kill_req.number)
          && (cur_com_cell->client_port == request_port)
          && (memcmp(&(cur_com_cell->client_host),
                     &request_host,
                     sizeof(soc_host)) == 0) ) {
            break;
        }
        cur_com_cell = cur_com_cell->prev;
      }
      if (cur_com_cell == NULL) {
        if (debug) {
          trace ("Cannot find command-host-port in list");
        }
        report.kill_rep.killed_pid = -1;
      } else {
        if (debug) {
          trace ("Pid to kill is %d", cur_com_cell->pid);
        }
        report.kill_rep.killed_pid = cur_com_cell->pid;
      }

      /* Kill the child */
      if ( (report.kill_rep.killed_pid != -1)
        && (kill(cur_com_cell->pid,
                 request_message.kill_req.signal_number) == -1) ) {
        char dbg[50];
        perror("kill");
        sprintf(dbg, "%d", cur_com_cell->pid);
        error("Cannot kill child pid: ", dbg);
        report.kill_rep.killed_pid = -1;
      }
    } else if (request_message.kind == start_command) {

      /* Start command */
      if (debug) {
        trace ("Request start %d: %s",
                request_message.start_req.number,
                request_message.start_req.command_text);
      }
      report.kind = start_report;
      report.start_rep.number = request_message.start_req.number;
      report.start_rep.started_pid =
             do_start_command(&request_message.start_req,
                              client_soc,
                              &request_host, request_port);

    } else if (request_message.kind == fexit_command) {

      /* Exit command */
      if (debug) {
        trace ("Request exit %d", request_message.fexit_req.exit_code);
      }
      report.kind = fexit_report;

    } else if (request_message.kind == ping_command) {

      /* Ping command */
      if (debug) {
        trace ("Request ping");
      }
      report.kind = pong_report;

    } else {
        error("Received a message with invalid command", "");
        continue;
    }

    /* Send report */
    if (! send(client_soc, (soc_message)&report, sizeof(report)) ) {
      perror("sending on socket");
      error("Cannot send exit report message", "");
    } else if (debug) {
      trace ("Start/Kill/Exit/Pong report sent\n");
    }

    if (request_message.kind == fexit_command) {
      if (debug) {
        trace ("Exiting code %d", request_message.fexit_req.exit_code);
      }
      exit(request_message.fexit_req.exit_code);
    }


  } /* Main loop */

}

