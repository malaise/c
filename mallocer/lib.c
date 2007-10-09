#include <pthread.h>

pthread_t th;

static void * the_thread (void * arg) {
  return arg;
}

extern void dummy (void);

extern void dummy (void) {
  (void) pthread_create (&th, NULL, &the_thread, NULL);
}

