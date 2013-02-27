/* Use /proc/pid/maps to measure memory heap allocated to a process,
   given its pid: a segment with empty path (entry with only 5 fields)
   and with perms (field 2) not "----".
   Field 1 is <start_addr>-<end_addr> so the segment size is the substraction.
   Sum the overall size of these segments (in bytes)
   Show <pid> <heap> <cmdline>
   On option (-d <delta>), show it each <delta> seconds (delta is int or float)
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <limits.h>
#include <time.h>

#include "timeval.h"
#include "get_line.h"

/* Max size of command line */
#define CMDLINE_SIZE 2014

/* Compute the heap size once */
static unsigned long hsize (pid_t pid) {
  char file_name[1024];
  char line[1024];
  char *addrs, *perms, *path;
  int i, got;
  FILE *file;
  unsigned long addr1, addr2;
  unsigned long size;

#define NEXT_SPACE while ((line[i] != ' ') && (line[i] != '\0')) i++
#define SKIP_SPACES while (line[i] == ' ') i++

  /* Open maps */
  sprintf(file_name, "/proc/%d/maps", pid);
  file = fopen (file_name, "r");
  if (file == NULL) {
    return 0;
  }

  /* Scan each line */
  size = 0;
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
    /* Skip offset, dev, inod */
    NEXT_SPACE; SKIP_SPACES;
    NEXT_SPACE; SKIP_SPACES;
    NEXT_SPACE;
    if (line[i] == '\0') {
      /* No path at all (usually the inode is followed by a space) */
      path = NULL;
    } else {
      SKIP_SPACES;
      path=&line[i];
    }
#ifdef DEBUG
      printf (">%s< >%s< >%s<\n", addrs, perms, path);
#endif

    /* Check if line matches: no path or [heap] and perms != '----' */
    if ( (path == NULL) || (strcmp (path, "") == 0)
                        || (strcmp (path, "[heap]") == 0) ) {
      if (strcmp (perms, "----") != 0) {
        /* OK, matches */
        sscanf (addrs, "%lx-%lx", &addr1, &addr2);
#ifdef DEBUG
        printf ("  %lx - %lx = %lx\n", addr1, addr2, addr2 - addr1);
#endif
        size += addr2 - addr1;
      }
    }
  }

  /* Done */
  fclose(file);
  return size;

}

/* Error and Usage */
static void error (const char *msg) __attribute__ ((noreturn));

static void error (const char *msg) {
  if ( (msg != NULL) && (strlen(msg) != 0) ) {
    fprintf(stderr, "ERROR: %s.\n", msg);
  }
  fprintf(stderr, "Usage: mheap [ -d <delta> ] <pid>\n");
  exit(1);
}

int main (int argc, char *argv[]) {
  /* The pid and command line of the scanned process */
  pid_t pid;
  float delta;
  char cmd_line[CMDLINE_SIZE];
  char file_name[1024];
  char msg[2048];
  FILE *file;
  unsigned long size;
  timeout_t curr_time, delta_time, next_time;
  char str_time[27];

  /* Parse arguments */
  if (argc == 2) {
    delta = 0.0;
  } else if ( (argc == 4) && (strcmp (argv[1], "-d") == 0) ) {
    delta = -1.0;
    sscanf (argv[2], "%f", &delta);
  } else {
    error("Invalid arguments");
  }
  if (delta < 0.0) {
    error ("Invalid delta");
  }

  /* Parse pid */
  pid = (pid_t) strtoul(argv[argc-1], NULL, 10);
  if ( (pid <= 0) || ((unsigned long) pid == ULONG_MAX) ) {
    error("Invalid <pid> value");
  }

  /* Get delta */
  double_to_time ((double) delta, &delta_time);

  /* Get command line (check pid is running) */
  sprintf(file_name, "/proc/%d/cmdline", pid);
  file = fopen (file_name, "r");
  if (file == NULL) {
    sprintf(msg, "Cannot open file %s, pid %d is probably not running",
            file_name, pid);
    error(msg);
  }
  fscanf (file, "%s", cmd_line);

  /* Loop on all outputs */
  get_time (&next_time);
  for (;;) {
    /* Get and put time and size */
    size = hsize(pid);
    if (size == 0) break;

    get_time(&curr_time);
    (void) image(&curr_time, str_time);
    str_time[23] = '\0';
    printf("%s %d %20lu %s\n", str_time, pid, size, cmd_line);

    /* Done if no iteration */
    if (delta <= 0.0) break;

    /* Sleep until next expiration */
    add_time (&next_time, &delta_time);
    wait_until (&next_time);
  }

  /* Done */
  exit(0);
}

