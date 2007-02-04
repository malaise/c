#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>

#include "sem_util.h"


union semun {
  int               val;    /* value for SETVAL */
  struct semid_ds   *buf;   /* buffer for IPC_STAT & IPC_SET */
  u_short           *array; /* array for GETALL & SETALL */
#ifdef linux
  struct seminfo *__buf;    /* buffer for IPC_INFO */
#endif
};

int get_sem_id (int sem_key) {
  int sem_id;

  sem_id = semget ( (key_t)sem_key, 1, 0666);
  if (sem_id == -1) {
    perror ("get_sem_id.semget");
    return (ERR);
  }

  return (sem_id);
}

t_result create_sem_key (int sem_key, int *p_sem_id) {
  int sem_id;
  union semun arg1;

  /* creation of the semaphore associated with the key key_sem */
  sem_id = semget ( (key_t)sem_key, 1, IPC_CREAT | 0666);
  if (sem_id  == -1) {
    perror ("create_sem_key.semget");
    return (ERR);
  }

  /* Out value sem_id */
  if (p_sem_id != NULL) *p_sem_id = sem_id;

  /* initialisation of the semaphore number 0 associated */
  /* with the id number semid */
  arg1.val = 1;
  if (semctl (sem_id, 0 , SETVAL , arg1) == -1 ) {
    perror ("create_sem_key.semctl");
    return (ERR);
  }

  return (OK);
}


t_result delete_sem_id (int sem_id) {
  /* deletion of the semaphore number 0 associated */
  /* with the id number semid */
  if (semctl (sem_id, 0 , IPC_RMID , 1) == -1 ) {
    perror ("delete_sem_id.semctl");
    return (ERR);
  }

  return (OK);
}


t_result delete_sem_key (int sem_key) {
  int sem_id;
  /* research of the id number associated with the key: key_sem */
  sem_id = get_sem_id (sem_key);
  if (sem_id  == ERR) return (ERR);

  return (delete_sem_id(sem_id));
}


static t_result incr_decr_sem_id (int sem_id, boolean incr, boolean undo) {
  struct sembuf sops;
  int res;

  sops.sem_num = (short) 0;  /* semaphore number 0 */
  sops.sem_op  = (short) (incr ? 1 : -1); /* semaphore operation */
  /* operation flags : process lock until the semaphore is zero */
  sops.sem_flg = (undo ? SEM_UNDO : 0);

  do {
    errno = 0;
    res = semop (sem_id, (struct sembuf *)(&sops), 1);
    if ( (res == -1) && (errno != EINTR) ) {
      perror ("decr_sem_id.semctl");
      return (ERR);
    }
  } while (res == -1);

  return (OK);
}

t_result decr_sem_id (int sem_id, boolean undo) {

  return incr_decr_sem_id (sem_id, FALSE, undo);
}


t_result decr_sem_key (int sem_key,  boolean undo) {
  int sem_id;

  sem_id = get_sem_id (sem_key);
  if (sem_id  == ERR) return (ERR);

  return decr_sem_id (sem_id, undo);
}



t_result incr_sem_id (int sem_id,  boolean undo) {
    return incr_decr_sem_id (sem_id, TRUE, undo);
}

t_result incr_sem_key (int sem_key,  boolean undo) {
  int sem_id;

  sem_id = get_sem_id (sem_key);
  if (sem_id  == -1) return (ERR);

  return incr_sem_id(sem_id, undo);
}
