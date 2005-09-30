#include <stdio.h>
#include <signal.h>
#include <sys/time.h>
#include <stdlib.h>

#include "sig_util.h"
#include "timeval.h"

static void sig_handler(int sig);

int main(void) {
  timeout_t timeout;

  set_handler (SIGTERM, sig_handler, NULL);
  set_handler (SIGINT, sig_handler, NULL);
  timeout.tv_sec =  5;
  timeout.tv_usec =  0;

  for (;;) {
    printf ("Running\n");
    delay (&timeout);
  }
  /* exit(0); */
}

static void sig_handler(int sig) {
  printf ("        signal received : %3d\n", sig);
}



