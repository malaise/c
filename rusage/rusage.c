#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/procfs.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <strings.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


#include "rusage.h"

/* Is the dump_rusage function already installed */
static int rusage_installed = 0;
/* File where file_block are written */
static FILE *rusage_file = (FILE*) NULL;

#ifdef alpha
/* Fd of our entry in /proc */
static int my_fd = -1;
#endif


/* Handler of RUSAGE_SIG signal */

#ifdef __STDC__
/*ARGSUSED*/
static void signal_rusage (int sig) 
#else
static void signal_rusage (sig) 
int sig;
#endif
{
  dump_rusage_str("Signal");
}

/* Inititialises the dump_rusage function */
/* Returns : RUSAGE_OK if success; RUSAGE_ERROR otherwise */
#ifdef __STDC__
int init_rusage(void) 
#else
int init_rusage() 
#endif /* __STDC__ */
{

  /* Hook dump_rusage signal handler on RUSAGE_SIG signal */
  if (signal (RUSAGE_SIG, signal_rusage) == SIG_ERR) {
    return (RUSAGE_ERROR);
  } else {
    return (RUSAGE_OK);
  }

}

#ifdef __STDC__
static int open_file(void) 
#else
static int open_file() 
#endif /* __STDC__ */
{
  /* Pid image on 5 digits */
  pid_t my_pid;

  char hostname[MAXHOSTNAMELEN];
  char file_name [255]; 

  if (rusage_installed != 0) return rusage_installed;

  /* Get pid */
  my_pid = getpid();

#ifdef alpha
  /* Open /proc/<pid> */
  {
    char proc_name[255];
    sprintf (proc_name, "/proc/%05d", my_pid);  
    my_fd = open(proc_name, O_RDONLY, 0);
  }
#endif

  /* Build file name : "rusage_HOSTNAME_PID" */
  if (gethostname(hostname, sizeof(hostname)) != 0) {
    return (RUSAGE_ERROR);
  }
  (void) sprintf (file_name, "rusage_%s_%05d", hostname, my_pid);

  /* Open file */
  rusage_file = fopen(file_name, "w");
  if (rusage_file == (FILE*)NULL) {
    /* Failed */
    rusage_installed = -1;
  } else {
    /* Done OK */
    rusage_installed = 1;
  }
  return rusage_installed;
}

#ifdef __STDC__
/*ARGSUSED*/
void dump_rusage (void)
#else
void dump_rusage ()
#endif
{
  dump_rusage_str(NULL);
}

#ifdef __STDC__
/*ARGSUSED*/
void dump_rusage_str (const char *str)
#else
void dump_rusage_str (str)
char *str;
#endif
{

  file_block block;

  /* One check on the status of the file on normal execution (file open) */
  if (open_file() != 1) {
    /* Failure in opening file. No action. */
    return;
  }

  /* Get time and rusage */
  gettimeofday (&block.time, NULL);
  getrusage(RUSAGE_SELF, &block.usage);

#ifdef alpha
  /* Get virtual size */
  {
    struct prpsinfo procinfo;
    block.pr_size = -1;
    if (my_fd != -1) {
      if (ioctl(my_fd, PIOCPSINFO, &procinfo) != -1) {
        block.pr_size = procinfo.pr_size;
      }
    }
  }
#else
  block.pr_size = 0;
#endif

  if (str == NULL) {
    block.msg[0] = '\0';
  } else {
    strncpy (block.msg, str, RU_USER_MSG_SIZE-1);
    block.msg[RU_USER_MSG_SIZE-1] =  '\0';
  }

  /* Write file_block on file */
  (void) fwrite ((void*)&block, sizeof(file_block), 1, rusage_file);
  (void) fflush (rusage_file);
}



