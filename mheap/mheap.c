/* Use /proc/pid/smaps to measure memory heap allocated to processes,
    given a pid list (pid,pid,pid...).
   The heap size is made of the segments with empty path or [heap] and with
    perms (field 2) not "---p".
   Field 1 is <start_addr>-<end_addr> so the segment size is the substraction.
   Sum the overall size of these segments (in bytes)
   On option (-a) count all segments (except ---p) of each pid, not only heap
   On option (-s), sum segments of all pids, counting read only shlibs only once
   On option (-d <delta>), show it each <delta> seconds (delta is int or float)
   Output is a list of <time> <pid> <size> [ <sum_size> ] [ <cmd_line> ]
     (<sum_size> if -s, <cmd_line> if -c)
     then (if -s) TOTAL <sum_size>
     possibly (if -d) repeated each <delta>
   <time> is YYYYMMDD-HHMMSS
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <limits.h>
#include <time.h>

#include "timeval.h"
#include "get_line.h"
#include "dynlist.h"

#define VERSION "mheap V2.0"

/* Max size of command line */
#define CMDLINE_SIZE 2014

/* Max number of pids */
#define MAX_PIDS 1020


/* LIST MANAGEMENT */
/*******************/
/* Max length of a shlib key (inode/ offset, 64bits eeach in hexa) */
#define KEY_LENGTH 34
typedef struct {
  char key[KEY_LENGTH];
} shlib_t;

/* List of shlibs seen so far (and already counted) */
dlist shlib_list;

/* Make a key */
static void make_key (char *inode, char *offset, shlib_t *result) {

  /* Make key : inode/ offset */
  strcpy (result->key, inode);
  strcat (result->key, "/");
  strcat (result->key, offset);
}

/* Is shlib already in list */
static boolean shl_in_list (char *inode, char *offset) {
  shlib_t crit, got;

  /* Make criteria : inode/ offset */
  make_key (inode, offset, &crit);

  /* Get list length */
  if (dlist_is_empty (&shlib_list)) {
    return FALSE;
  }

  /* Search matching */
  dlist_rewind (&shlib_list, TRUE);
  for (;;) {
    dlist_read (&shlib_list, &got);
    if (strcmp (crit.key, got.key) == 0) {
      /* Found it */
      return TRUE;
    }
    /* Done if end of list */
    if (dlist_get_pos(&shlib_list, FALSE) == 1) {
      return FALSE;
    }
    /* Move to next */
    dlist_move (&shlib_list, 1);
  }
}

/* Add shlib to list */
static void add_to_list (char *inode, char *offset) {
  shlib_t shlib;

  /* Make key : inode/ offset */
  make_key (inode, offset, &shlib);

  /* Insert in head */
  dlist_rewind (&shlib_list, TRUE);
  dlist_insert (&shlib_list, &shlib, TRUE);
}


/* HEAP OF A PROC */
/******************/
/* Compute the heap size once */
static void hsize (pid_t pid, int show_all, int show_sum,
                   unsigned long *size, unsigned long *sum_size) {
  char file_name[1024];
  char line[1024];
  char *addrs, *perms, *offset, *inode, *path;
  int i, got, len;
  boolean is_shlib = FALSE, sum_it = FALSE, sum_dirty = FALSE;
  FILE *file;
  unsigned long addr1 = 0L, addr2 = 0L, dirty_size = 0L;

#define NEXT_SPACE while ((line[i] != ' ') && (line[i] != '\0')) i++
#define SKIP_SPACES while (line[i] == ' ') i++

  *size = 0;
  *sum_size = 0;
  /* Open smaps */
  sprintf(file_name, "/proc/%d/smaps", pid);
  file = fopen (file_name, "r");
  if (file == NULL) {
    return;
  }

  /* Scan each line */
#ifdef DEBUG
  printf ("Pid %d\n", pid);
#endif
  for (;;) {
    got = get_line (file, line, sizeof(line));
    /* end of file? */
    if (got == -1) break;

    /* Parse addresses */
    i = 0;
    addrs = line;
    NEXT_SPACE;
    line[i] = '\0';
    i++;
    SKIP_SPACES;
    /* Parse perms */
    perms=&line[i];
    NEXT_SPACE;
    line[i] = '\0';
    i++;
    SKIP_SPACES;
    /* Parse offset*/
    offset=&line[i];
    NEXT_SPACE;
    line[i] = '\0';
    i++;
    SKIP_SPACES;
    /* Skip dev */
    NEXT_SPACE; SKIP_SPACES;
    /* Parse inod */
    inode=&line[i];
    NEXT_SPACE;
    /* Optional path */
    if (line[i] == '\0') {
      /* No path at all (usually the inode is followed by a space anyway) */
      path = NULL;
    } else {
      line[i] = '\0';
      i++;
      SKIP_SPACES;
      path=&line[i];
    }
#ifdef DEBUG
    printf ("Addr>%s< Perms>%s< Off>%s< Inode>%s< Path>%s<\n",
            addrs, perms, offset, inode, path);
#endif

    /* Get details from smap - 13 fields*/
    int DETAIL_FIELDS = 13;
    typedef enum fields_names { SIZE, RSS, PSS, SHARED_CLEAN, SHARED_DIRTY, PRIVATE_CLEAN, PRIVATE_DIRTY, REFERENCED, ANONYMOUS, ANON_HUGE_PAGES, SWAP, KERNEL_PAGE_SIZE, MMU_PAGE_SIZE } Field_names;
    int j = 0, got_details = 0, k = 0;
    char detail_line[1024];
    for( j= 0; j < DETAIL_FIELDS; j++ ) {
        got_details = get_line (file, detail_line, sizeof(detail_line));
        if (got == -1) break; /* Should never happen */
        if( PRIVATE_DIRTY == j ) {
            /* Move to start of value*/
            while ((detail_line[k] != ' ') && (detail_line[k] != '\0')) k++;
            while (detail_line[k] == ' ') k++;
            /* Copy the value, cutting out the "kB" unit measure "*/
            char * temp;
            temp = &detail_line[k];
            sscanf (temp, "%lu kB", &dirty_size);
#ifdef DEBUG
            printf ("Private_Dirty size >%ld<\n", dirty_size);
#endif
        }
    }

    /* Do we count this segment? */
    if (strcmp (perms, "---p") == 0) {
      /* Not accessible, discard */
      continue;
    } else {
      /* Check if line matches: all or no path or [heap] */
      if ( !show_all && (path != NULL) && (strcmp (path, "") != 0)
           && (strcmp (path, "[heap]") != 0) ) {
        continue;
      }
    }

    /* Do we sum this segment */
    if (show_sum) {
      /* In sum mode, count all segments except RO shlibs that are counted */
      /*  only once */
      is_shlib = FALSE;
      len = strlen (path);
      if ( (len >= 3) && (strcmp (&path[len-3], ".so") == 0) ) {
        /* This is a shlib (path ends by ".so") */
        is_shlib = TRUE;
      } else if (strstr (path, ".so.") != NULL) {
        /* This is a shlib (xxx.so.i.j.k) */
        is_shlib = TRUE;
      }
      if (is_shlib) {
        if (! shl_in_list (inode, offset)) {
          /* This shlib is unknown, add it and count it */
#ifdef DEBUG
          printf ("  Adding shlib %s\n", path);
#endif
          add_to_list (inode, offset);
          sum_it = TRUE;
        } else {
            if (strcmp (perms, "rw-p") == 0) {
#ifdef DEBUG
              printf ("  Adding modified pages of writable shlib %s\n", path);
#endif
              sum_dirty = TRUE;
            } else {    
#ifdef DEBUG
              printf ("  Skipping shlib %s\n", path);
#endif
              sum_it = FALSE;
            }
        }
      } else {
        sum_it = TRUE;
      }
    }

    /* Count this size */
    sscanf (addrs, "%lx-%lx", &addr1, &addr2);
#ifdef DEBUG
    printf ("  %lx - %lx = %lx", addr1, addr2, addr2 - addr1);
#endif
    *size += addr2 - addr1;
    if (show_sum && sum_dirty) {
      *sum_size += dirty_size*1024L;
#ifdef DEBUG
      printf (" and SUM dirty\n");
#endif
    } else if (show_sum && sum_it) {
      *sum_size += addr2 - addr1;
#ifdef DEBUG
      printf (" and SUM\n");
#endif
    } else {
#ifdef DEBUG
      printf ("\n");
#endif
    }

    /* Reset what to sum flags */
    sum_it = FALSE;
    sum_dirty = FALSE;

  } /* Loop on all segments */

  /* Done */
  fclose(file);

}


/* ERROR AND USAGE */
/*******************/
static void error (const char *msg) __attribute__ ((noreturn));

static void error (const char *msg) {
  if ( (msg != NULL) && (strlen(msg) != 0) ) {
    fprintf(stderr, "ERROR: %s.\n", msg);
  }
  fprintf(stderr, "Usage: mheap [ -a ] [ -d <delta> ] <pid>\n");
  exit(1);
}

static void help (void) {
  printf ("Usage: mheap [ <options> ] <pid_list>\n");
  printf ("   or: mheap -v | -h\n");
  printf (" <pid_list> ::= <pid>[{,<pid>}]\n");
  printf (" <options>  ::= -c | -a | -d <delta> | -s\n");
  printf (" Lists each pid and corresponding head size and exists\n");
  printf (" -c => show command line after each size\n");
  printf (" -a => count all segments (except ---p) of each pid\n");
  printf (" -d => loop and display each <delta> seconds\n");
  printf (" -s => shows and sums total size of pids, counting shlibs only once\n");
}


/* MAIN */
/********/
int main (int argc, char *argv[]) {
  /* Indexes in arguments and pid list */
  int i, j;
  /* Argument pid list and its length */
  char *pids_arg;
  int len;
  /* The pids of the scanned processes */
  int nb_pids;
  pid_t pids[MAX_PIDS];
  /* Show command line */
  boolean show_cmd;
  /* Delta between measures */
  float delta;
  /* Count all segments */
  boolean show_all;
  /* Show total (sum) size */
  boolean show_sum;
  /* Command line of the process */
  char cmd_line[CMDLINE_SIZE];
  /* File in /proc of cmd line */
  char file_name[1024];
  FILE *file;
  /* Error message */
  char msg[2048];
  /* Vsize of the process */
  unsigned long size;
  /* Total vsize of the process, of the processes */
  unsigned long sum_size;
  unsigned long total_size;
  /* Times */
  timeout_t curr_time, delta_time, next_time;
  struct tm curr_tm;
  /* Displayed time YYYYMMDD-HHMMSS*/
  char str_time[16];

  /* Parse arguments */
  if (argc == 2) {
    if (strcmp (argv[1], "-h") == 0) {
      help();
      exit (1);
    } else if (strcmp (argv[1], "-v") == 0) {
      printf ("%s\n", VERSION);
      exit (1);
    }
  } else if (argc == 1){
      help();
      exit (1);
  }
  delta = 0.0;
  show_all = FALSE;
  show_cmd = FALSE;
  show_sum = FALSE;
  for (i = 1; i < argc - 1; i++) {
    if (strcmp (argv[i], "-d") == 0) {
      if (delta > 0.0) {
        /* Re definition of -d */
        error ("Invalid arguments");
      }
      i++;
      if (i < argc - 1) {
        delta = -1.0;
        sscanf (argv[i], "%f", &delta);
        if (delta < 0.0) {
          error ("Invalid delta");
        }
      } else {
        /* no pid */
        error ("Invalid arguments");
      }
    } else if (strcmp (argv[i], "-a") == 0) {
      if (show_all) {
        /* Re definition of -a */
        error ("Invalid arguments");
      }
      show_all = TRUE;
    } else if (strcmp (argv[i], "-c") == 0) {
      if (show_cmd) {
        /* Re definition of -c */
        error ("Invalid arguments");
      }
      show_cmd = TRUE;
    } else if (strcmp (argv[i], "-s") == 0) {
      if (show_sum) {
        /* Re definition of -s */
        error ("Invalid arguments");
      }
      show_sum = TRUE;
    } else {
      error ("Invalid arguments");
    }
  }
  if (i != argc - 1) {
    error ("Invalid arguments");
  }


  /* Parse pids */
  nb_pids = 0;
  i = 0;
  pids_arg = argv[argc-1];
  len = strlen(pids_arg);
  /* Each pid */
  for (j = 0;; j++) {
    if (pids_arg[j] == ',') {
      /* Separator between pids */
      pids_arg[j] = '\0';
    }
    if (pids_arg[j] == '\0') {
      /* Separator or end of argument: get pid and check */
      pids[nb_pids] = (pid_t) strtoul(&(pids_arg[i]), NULL, 10);
      if ( (pids[nb_pids] <= 0)
        || ((unsigned long) pids[nb_pids] == ULONG_MAX) ) {
        sprintf(msg, "Invalid pid %s", &(pids_arg[i]));
        error(msg);
      }
      /* Pid got OK */
      nb_pids ++;
      /* Mark start of next pid */
      i = j + 1;
      /* Stop and end of list */
      if (j == len) {
        break;
       }
    }
  }

  /* Get delta */
  double_to_time ((double) delta, &delta_time);


  /* Loop on all periodic outputs */
  get_time (&next_time);
  /* Init dynamic list  of shlibs */
  dlist_init (&shlib_list, sizeof (shlib_t));
  for (;;) {
    /* Loop on all pids */
    dlist_delete_all (&shlib_list);
    total_size = 0;

    /* A small header */
#ifdef DEBUG
    printf ("%s\t\t%s\t%20s", "Time", "Pids", "Size");
    if (show_sum) {
        printf ("\t%20s", "Size to sum");
    }
    if (show_cmd) {
        printf ("\t%s", "Command line");
    }
    printf("\n"); 
#endif

    for (i = 0; i < nb_pids; i++) {

      /* Get command line (check pid is running) */
      sprintf(file_name, "/proc/%d/cmdline", pids[i]);
      file = fopen (file_name, "r");
      if (file == NULL) {
        sprintf(msg, "Cannot open file %s, pid %d is probably not running",
                file_name, pids[i]);
        error(msg);
      }
      fscanf (file, "%s", cmd_line);

      /* Get and put time and size */
      hsize(pids[i], show_all, show_sum, &size, &sum_size);
      if (size == 0) break;

      get_time(&curr_time);
      (void)gmtime_r(&(curr_time.tv_sec), &curr_tm);
      strftime (str_time, 16, "%Y%m%d-%H%M%S.", &curr_tm);
      printf ("%s\t%d\t%20lu", str_time, pids[i], size);
      if (show_sum) {
        printf ("\t%20lu", sum_size);
      }
      if (show_cmd) {
        printf ("\t%s", cmd_line);
      }
      printf ("\n");
      total_size += sum_size;
    }

    /* Total for all pids */
    if (show_sum) {
      printf("%s\tTOTAL\t%44lu\n", str_time, total_size);
    }

    /* Done if no iteration */
    if (delta <= 0.0) break;
    printf("\n");

    /* Sleep until next expiration */
    add_time (&next_time, &delta_time);
    wait_until (&next_time);
  }

  /* Done */
  exit(0);
}

