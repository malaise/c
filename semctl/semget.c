#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

#include "sem_util.h"
#include "timeval.h"

#define KEY 0x11111

int main (void) {

  int id;
  int wait;
  timeout_t timeout;

  printf ("My pid is %d\n", getpid());

  id = get_sem_id (KEY);
  
  if (id != ERR) {
    printf ("Sem %d got, id %d\n", KEY, id);
    wait = 0;
  } else {
    fflush(stderr);
    if (create_sem_key (KEY, &id) == OK) {
      printf ("Sem %d created, id %d\n", KEY, id);
    } else {
      printf ("sem %d creation failure. Errno %d\n", KEY, errno);
      exit (1);
    }
    wait = 1;
  }
 
  if (decr_sem_id(id, FALSE) != OK) {
    printf ("Sem %d id %d decrementation failure. Errno %d\n", KEY, id, errno);
  }

  if (wait) {
    for (;;) {
      timeout.tv_sec = 10;
      timeout.tv_usec = 0;
      delay (&timeout);
    }
  }
  exit(0);
}

