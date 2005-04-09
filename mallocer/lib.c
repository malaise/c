#include <pthread.h>

extern void dummy (void);

extern void dummy (void) {
  (void) pthread_create (NULL, NULL, NULL, NULL);
}

