#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <stdlib.h>

#include "sig_util.h"

#define ALRM_S 000005
#define ALRM_U 000000

extern void  sigpause (int mask);

static void handler_sigalrm(int sig);
static void handler_sigusr1(int sig);
static void handler_sigusr2(int sig);

int main(void) {

  printf ("My pid is %d\n", getpid());
  printf (
   "Signal managed : SIGTALRM(intern), SIGUSR1(30), SIGUSR2(31)\n");


  set_handler (SIGALRM, handler_sigalrm, NULL);
  set_handler (SIGUSR1, handler_sigusr1, NULL);
  set_handler (SIGUSR2, handler_sigusr2, NULL);

  printf ("Start timer\n");
  arm_timer (ITIMER_REAL, (long)ALRM_S, (long)ALRM_U, 1);

  sigpause (sigmask(SIGALRM) | sigmask(SIGUSR1));
  printf ("Done.\n");
  exit(0);
}

static void handler_sigalrm (int sig) {
  printf ("        SIGALRM...%3d\n", sig);
}

static void handler_sigusr1 (int sig) {
  printf ("        SIGUSR1...%3d\n", sig);
}

static void handler_sigusr2 (int sig) {
  printf ("        SIGUSR2...%3d\n", sig);
}



