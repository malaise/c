#include "stdio.h"
#include "stdlib.h"
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "malloc.h"

static unsigned long allocated;

static void *my_malloc(size_t size) {

  allocated = allocated + size;
  return malloc(size);
}


static size_t put_stats (int put_stats) {
  struct mallinfo2 info;
  pid_t pid;
  FILE *file;
  char buffer[1024];
  char vmtext[1024];
  const char *crit = "VmSize:";
  long int vmsize, mfree;

  /* Get virtual size from /proc/pid/stat line "VmSize:" */
  pid = getpid();
  sprintf(buffer, "/proc/%d/status", (int)pid);
  file = fopen(buffer, "r");
  if (file == NULL) {
    perror("fopen");
    exit(1);
  }
  for (;;) {
    if (fscanf(file, "%s%s", buffer, vmtext) <= 0) {
      perror("scanf");
      exit(1);
    }
    if (strncmp (buffer, crit, strlen(crit) + 1) == 0) break;
  }
  fclose (file);
  vmsize = strtol (vmtext, NULL, 10);

  /* Mallinfo */
  info = mallinfo2();
#ifdef DEBUG
  printf("Mallinfo\n");
  printf("Arena %d\n", info.arena);
  printf("Free chunks %d\n", info.ordblks);
  printf("Fastbin blocks %d\n", info.smblks);
  printf("Mmapped regions %d\n", info.hblks);
  printf("Space in mmapped regions %d\n", info.hblkhd);
  printf("Max allocated space %d\n", info.usmblks);
  printf("Space available in freed fastbin blocks %d\n", info.fsmblks);
  printf("Total allocated space %d\n", info.uordblks);
  printf("Total free space %d\n", info.fordblks);
  printf("Top-most, releasable space %d\n", info.keepcost);
#endif

  /* Malloc_stats */
  if (put_stats) {
    malloc_stats();
  }

  mfree = (long)info.fordblks;
  return (size_t)vmsize * 1024 - mfree;
}

#define MAX_MALLOC (10 * 1024 * 1024)
#define MAX_LEAK (10 * 1024)
#define NB_SLOTS 100
void *slots[NB_SLOTS];
static void alloc (int leak) {
  int i;
  size_t sizer;
  char *p;

  sizer = random();
  /* A hundredth of allocs leak */
   if (leak && (random() < RAND_MAX / 100)) {
    /* Leak by less than 10KB, store amount */
    sizer = (size_t) ((float)sizer / (float)RAND_MAX * (float)MAX_LEAK);
    p = my_malloc(sizer);
  } else {
    /* Malloc less than 3MB, free first */
    sizer = (size_t) ((float)sizer / (float)RAND_MAX * (float)MAX_MALLOC);
    i = (int) ((float)sizer / (float)RAND_MAX * (float)NB_SLOTS);
    if (i >= NB_SLOTS) i = NB_SLOTS - 1;
    if (slots[i] != NULL) free(slots[i]);
    p = malloc(sizer);
    slots[i] = p;
  }
  if (p == NULL) {
    perror("malloc");
    exit(1);
  }

}


int main (int argc, char *argv[]) {

  int leak;
  int j, i;
  struct timeval tv;
  size_t size1, size2;
  long delta;
  char *p;


  /* Init random */
  printf ("My pid is %d\n", getpid());
  leak = ((argc == 2) && (strcmp (argv[1], "leak") == 0));
  if (leak) printf ("Leaking...\n");
  sleep (5);
  gettimeofday (&tv, NULL);
  srandom (tv.tv_usec);

  /* Init slots */
  for (i = 0; i < NB_SLOTS; i++) slots[i] = NULL;

  /* 128M allocated and freed to set brk/mmap thresholdt,
   * then 1GB allocated, 
   * then one alloc to tare
   */
  printf ("Start\n");
  size1 = put_stats(0);
  printf ("\n");
  p = malloc(128 * 1024 * 1024);
  free(p);
  p = malloc (1024 * 1024 * 1024);
  alloc(leak);
  allocated = 0L;

  /* Initial (reference) state */
  printf ("Initial\n");
  size1 = put_stats(1);
  printf ("\n");

  for (j = 0; j < 100; j++) {
    /* Loop on malloc and leaks */
    for (i = 0; i < 5000; i++) {
      alloc(leak);
    }
    size2 = put_stats(0);
    if (i == 0) size1 = size2;
    sleep(2);
  }

  /* Final */
  /* Garbage-collect mallocs that are not leaks */
  printf("Final\n");
  for (i = 0; i < NB_SLOTS; i++) {
    if (slots[i] != NULL) free(slots[i]);
  }
  size2 = put_stats(1);

  /* End to end stats */
  delta = (long)size2-(long)size1;
  printf("Delta size %ld\n", delta);
  printf("Allocated %lu\n", allocated);
  printf("Error %ld\n", delta-(long)allocated);

  sleep(5);
  exit(0);
} 

