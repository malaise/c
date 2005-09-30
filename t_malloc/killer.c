#include <sys/types.h>
#include <signal.h>
#include <stdlib.h>

int main (int argc, char *argv[]) {

  int pid = atoi(argv[1]);

  if (argc == 2) {
    pid = atoi(argv[1]);

    for (;;) {
      kill (pid, SIGUSR1);
    }
  }
  exit(0);
}

