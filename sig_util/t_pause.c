#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "sig_util.h"

#define ALRM_S 000005
#define ALRM_U 000000

static void handler_sigalrm(int signum);
static void handler_sigusr1(int signum);
static void handler_sigusr2(int signum);


int main (void) {

  sigset_t mask;


  printf ("My pid is %d\n", getpid());
  printf (
   "Signal managed : SIGTALRM(intern), SIGUSR1(30), SIGUSR2(31)\n");


  set_handler (SIGALRM, handler_sigalrm, NULL);
  set_handler (SIGUSR1, handler_sigusr1, NULL);
  set_handler (SIGUSR2, handler_sigusr2, NULL);

  printf ("Start timer\n");
  arm_timer (ITIMER_REAL, (long)ALRM_S, (long)ALRM_U, 1);

  sigaddset (&mask, SIGALRM);
  sigaddset (&mask, SIGUSR1);
  sigsuspend (&mask);
  printf ("Done.\n");
  exit(0);
}

void handler_sigalrm (int signum) {
  printf ("        SIGALRM...%3d\n", signum);
}

void handler_sigusr1 (int signum) {
  printf ("        SIGUSR1...%3d\n", signum);
}

void handler_sigusr2 (int signum) {
  printf ("        SIGUSR2...%3d\n", signum);
}



