#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>


union semun {
  int val;
  struct semid_ds *buf;
  short *array;
};

static void usage(void) __attribute__ ((noreturn));
static void usage(void) {
  fprintf (stderr, "Usage : semctl -k key [ semnum ]    or    semctl -i id [ semnum ]\n");
  fprintf (stderr, "   semnum is a number or b or s\n");
  exit (2);
}

/* Print access rights on semaphore */
static void print_rights (int rights, int mask_read, int mask_alter) {
  if ( (rights & (mask_read | mask_alter) ) == 0 ) {
    printf ("--");
  } else {
    if ( (rights & mask_read) == mask_read ) {
      printf ("r");
    } else {
      printf (" ");
    }
    if ( (rights & mask_alter) == mask_alter ) {
      printf ("a");
    } else {
      printf (" ");
    }
  }
}

int main (int argc, char *argv[]) {
  int i;
  unsigned int u;
  int semid;
  int semnum, nsemnum, fsemnum;
  int blocked, say;
  int some_wait;
  union semun arg;
  int val, pid, ncnt, zcnt, res, rights;
  char key[3];
  /* String of the date printed : year month day hours:min:sec */
  char printed_date [133];
  char *date;
  struct passwd * pw;
  struct group *  gr;

  arg.buf = (struct semid_ds*) malloc (sizeof(struct semid_ds));


  if ( (argc != 3) && (argc != 4) ) usage();
  if (strlen(argv[1]) >= sizeof(key) ) usage();
  strcpy(key, argv[1]);
  for (i = 0; (unsigned)i < strlen(key); i++) {
    key[i] = toupper(key[i]);
  }


  if ( (strcmp (key, "-K") != 0) &&
       (strcmp (key, "-I") != 0) ) usage();

  i = 0;
  i = atoi(argv[2]);
  if ( (i == 0) && (strcmp (argv[2], "0") != 0 ) ) {
    if (strcmp (key, "-I") == 0) { 
      fprintf (stderr, "Bad semid\n");
      usage();
    }
    if ( (argv[2][0] == '0') && (argv[2][1] == 'x') ) {
      if (sscanf (argv[2], "%x", &u) <= 0) {
        fprintf (stderr, "Bad semkey\n");
        usage();
      }
      i = (int)u;
    } else {
      fprintf (stderr, "Bad semkey\n");
      usage();
    }
  }

  if (strcmp (key, "-K") == 0) { 
    if (i == 0) {
      fprintf (stderr, "Bad semkey\n");
      usage();
    }
    semid = semget ( (key_t)i, 1, 0);
    if (semid == -1) {
      perror ("semget");
      fprintf (stderr, "Cannot find sem with key %d\n", i);
      usage();
    }
  } else {
    semid = i;
  }

  if (argc == 3) {
    nsemnum = -1;
  } else if ( (strcmp (argv[3], "b") == 0)
           || (strcmp (argv[3], "B") == 0) ) {
    nsemnum = -2;
  } else if ( (strcmp (argv[3], "s") == 0)
           || (strcmp (argv[3], "S") == 0) ) {
    nsemnum = -3;
  } else if ( strcmp (argv[3], "0") == 0) {
    nsemnum = 0;
  } else {
    nsemnum = atoi (argv[3]);
    if (nsemnum == 0){
      printf ("Bad semnum\n");
      usage();
    }
  }

  res = semctl (semid, 0, IPC_STAT, arg);
  if (res == -1) {
    perror ("semctl, stat");
    fprintf (stderr, "Cannot access to sem id %d\n", semid);
    exit(2);
  }

  rights =  (int) (arg.buf->sem_perm).mode;
  if (nsemnum == -1) {
    fsemnum = 0;
    nsemnum = arg.buf->sem_nsems - 1;
    blocked = 0;
    say = 1;
    printf ("All on ");
  } else if (nsemnum == -2) {
    fsemnum = 0;
    nsemnum = arg.buf->sem_nsems - 1;
    blocked = 1;
    say = 1;
    printf ("Blocked on ");
  } else if (nsemnum == -3) {
    fsemnum = 0;
    nsemnum = arg.buf->sem_nsems - 1;
    blocked = 1;
    say = 0;
  } else {
    fsemnum = nsemnum;
    blocked = 0;
    say = 1;
    printf ("No %d of ", nsemnum);
  }
  
  if (say) {
  printf ("Semaphore ");
    if (strcmp (key, "-K") == 0) { 
      printf ("Key %d id %d, ", i, semid);
    } else {
      printf ("Id %d, ", semid);
    }

    printf ("num in 0 .. %ld\n", arg.buf->sem_nsems - 1);

    pw = getpwuid((arg.buf->sem_perm).cuid);
    gr = getgrgid((arg.buf->sem_perm).cgid);
    printf ("Created by %s group %s\n", pw->pw_name, gr->gr_name);
    pw = getpwuid((arg.buf->sem_perm).uid);
    gr = getgrgid((arg.buf->sem_perm).gid);
    printf ("Owned   by %s group %s\n", pw->pw_name, gr->gr_name);

    printf ("Access rights User : ");
    print_rights (rights, 0x100, 0x80);
    printf ("   Group : ");
    print_rights (rights, 0x20, 0x10);
    printf ("   Others : ");
    print_rights (rights, 0x4, 0x2);
    printf ("\n");


    printf ("+---------------------------------------------------------------------------------------+\n");
    printf ("| Num | Val | LastPid |       LastDateUpdate |       LastDateChange | Nproc>0 | Nproc=0 |\n");
    printf ("|-----+-----+---------+----------------------+----------------------+---------+---------|\n");
  }

  some_wait = 0;
  for (semnum = fsemnum; semnum <= nsemnum; semnum++) {

    val = semctl (semid, semnum, GETVAL, arg);
    if (val == -1) {
      perror ("semctl, getval");
      fprintf (stderr, "Cannot access to sem id %d\n", semid);
      exit (2);
    }
    pid = semctl (semid, semnum, GETPID, arg);
    if (pid == -1) {
      perror ("semctl, getpid");
      fprintf (stderr, "Cannot access to sem id %d\n", semid);
      exit (2);
    }
    ncnt = semctl (semid, semnum, GETNCNT, arg);
    if (ncnt == -1) {
      perror ("semctl, getncnt");
      fprintf (stderr, "Cannot access to sem id %d\n", semid);
      exit (2);
    }
    zcnt = semctl (semid, semnum, GETZCNT, arg);
    if (zcnt == -1) {
      perror ("semctl, zcnt");
      fprintf (stderr, "Cannot access to sem id %d\n", semid);
      exit (2);
    }

    some_wait = some_wait || ((ncnt + zcnt) != 0);
    if (  ( (!blocked) || ((ncnt + zcnt) != 0)) && say) {

      printf ("| %3d | ", semnum);
      printf ("%3d | ", val);
      printf ("%7d | ", pid);
  
      date = ctime (&(arg.buf->sem_otime));
      (void) strncpy (&(printed_date[0]), &(date[20]), 4);
      printed_date[4] = ' ';
      (void) strncpy (&(printed_date[5]), &(date[4]), 15);
      printed_date[20] = '\0';
      printf ("%s | ", printed_date);

      date = ctime (&(arg.buf->sem_ctime));
      (void) strncpy (&(printed_date[0]), &(date[20]), 4);
      printed_date[4] = ' ';
      (void) strncpy (&(printed_date[5]), &(date[4]), 15);
      printed_date[20] = '\0';
      printf ("%s | ", printed_date);

      printf ("%7d | ", ncnt);
      printf ("%7d |\n", zcnt);
    }

  }
  if (say) printf ("+---------------------------------------------------------------------------------------+\n");

  if (some_wait) {
    exit(1);
  } else {
    exit(0);
  }
}

