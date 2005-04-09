#include "boolean.h"

/* Results of calls */
#define OK 0
#define ERR -1

#define t_result int

/* Creates a semaphore associated with the given key */
/* The out value sem_id is set, if p_sem_id != NULL for furhter use */
t_result create_sem_key (int sem_key, int *p_sem_id);

/* Id of the semaphore associated with the key, or ERR */
/* The sem must have been created */
int get_sem_id (int sem_key);

/* The calls with id are faster the the calls with the key */
t_result delete_sem_id (int sem_id);
t_result delete_sem_key (int sem_key);

/* decrements the semaphore (try to get (blocking) ) */
t_result decr_sem_id (int sem_id, boolean undo);
t_result decr_sem_key (int sem_key, boolean undo);

/* increments (release) the semaphore */
t_result incr_sem_id (int sem_id, boolean undo);
t_result incr_sem_key (int sem_key, boolean undo);

