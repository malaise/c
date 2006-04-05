#include <stdlib.h>
#include <errno.h>

#define __USE_GNU
#include "mutex.h"

/* Create a mutex */
extern mutex_t mutex_create(void) {
  mutex_t mutex;
  pthread_mutex_t mut = PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP;

  /* Allocate mutex */
  mutex = malloc(sizeof(pthread_mutex_t));
  if (mutex == NULL) {
    return NULL;
  }

  /* Init mutex */
  *mutex = mut;

  return mutex;
}


/* Destroy a mutex */
/* Returns MUT_LOCKED if the mutex is locked */
/* Else returns MUT_OK */
extern int mutex_destroy (mutex_t mutex) {
  int res;

  /* Destroy mutex */
  res = pthread_mutex_destroy(mutex);

  switch (res ) {
    case 0 : break;
    case EBUSY : return MUT_LOCKED;
    default : return MUT_ERROR;
  }

  /* Free mutex*/
  free (mutex);
  return MUT_OK;
}


/* Get a mutex */
/* Returns MUT_DEADLOCK if this thread already owns this mutex */
/* If wait is set to TRUE, wait for mutex and return MUT_OK */
/* Else, may return MUT_OK or MUT_LOCKED */
extern int mutex_lock (mutex_t mutex, boolean wait) {
  int res;

  if (wait) {
    res = pthread_mutex_lock (mutex);
  } else {
    res = pthread_mutex_trylock (mutex);
  }

  switch (res ) {
    case 0 : return MUT_OK;
    case EINVAL : return MUT_ERROR;
    case EDEADLK : return MUT_DEADLOCK;
    case EBUSY : return MUT_LOCKED;
    default : return MUT_ERROR;
  }
}


/* Release a mutex */
/* Returns MUT_NOTLOCKED if not locked by this thread (or not locked at all) */
/* Else returns MUT_OK */
extern int mutex_unlock (mutex_t mutex) {
  int res;

  res = pthread_mutex_unlock (mutex);

  switch (res) {
    case 0 : return MUT_OK;
    case EINVAL : return MUT_ERROR;
    case EPERM : return MUT_DEADLOCK;
    default : return MUT_ERROR;
  }
}

