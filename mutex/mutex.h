#include <pthread.h>

#include "boolean.h"

#define MUT_OK         (0)
#define MUT_LOCKED    (-1)
#define MUT_DEADLOCK  (-2)
#define MUT_NOTLOCKED (-3)
#define MUT_ERROR     (-4)

typedef pthread_mutex_t *mutex_t;

/* All calls excep create may raise MUT_ERROR */
/* If mutex not init or destroyed or... */

/* Create a mutex */
/* Returns NULL if malloc error */
extern mutex_t mutex_create(void);

/* Destroy a mutex */
/* Returns MUT_LOCKED if the mutex is locked */
/* Else returns MUT_OK */
extern int mutex_destroy (mutex_t mutex);


/* Get a mutex */
/* Returns MUT_DEADLOCK if this thread already owns this mutex */
/* If wait is set to TRUE, wait for mutex and return MUT_OK */
/* Else, may return MUT_OK or MUT_LOCKED */
extern int mutex_lock (mutex_t mutex, boolean wait);

/* Release a mutex */
/* Returns MUT_NOTLOCKED if not locked by this thread (or not locked at all) */
/* Else returns MUT_OK */
extern int mutex_unlock (mutex_t mutex);

