
#ifndef __RUSAGE_H__
#define __RUSAGE_H__ 1

#include <sys/time.h>
#include <sys/resource.h>
#include <signal.h>

/* Signal which makes the rusage to be dumped */
# define RUSAGE_SIG SIGXFSZ

/* Result */
#define RUSAGE_OK        0
#define RUSAGE_ERROR    -1

#ifdef __STDC__

/* First call to initialise the rusage procedure */
/* Need to be called once before triggering dump_rusage with signal */
/*  RUSAGE_SIG */
/* Returns RUSAGE_OK, or RUSAGE_ERROR. */
extern int init_rusage(void);

/* Call to dump a record, possibly with a title */
extern void dump_rusage(void);
extern void dump_rusage_str(const char *);

#else /* !__STDC__ */

extern int init_rusage();
extern void dump_rusage();
extern void dump_rusage_str();

#endif /* __STDC__ */

/* Structure dumped in the file "rusage_HOST_PID" */
/* At each time the RUSAGE_SIG signal is received or dump_rusage is called */
/* and to be read by anal_rusage program */

#define RU_USER_MSG_SIZE 128

typedef struct {
  struct timeval time;
  struct rusage  usage;
  long   pr_size;
  char   msg[RU_USER_MSG_SIZE];
} file_block;

#endif /* __RUSAGE_H__ */

