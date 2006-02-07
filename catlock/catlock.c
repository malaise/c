#include <stdio.h>
#include <libgen.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

static void Close (int fd) {
  if (fd != 0) (void) close (fd);
}

#define BLK_SZ 1024

int main (int argc, char *argv[]) {

  char *me = basename (argv[0]);
  int sfd, dfd;
  int res;
  off_t offset;
  size_t rsize, wsize;
  char buffer[BLK_SZ];
  char *ifm, *ofm;

  /* 1 arg : dest file */
  /* 2 args: source file, dest file */
  if (argc == 2) {
    ifm = NULL;
    ofm = argv[1];
  } else if (argc == 3) {
    ifm = argv[1];
    ofm = argv[2];
  } else {
    fprintf (stderr, "Usage: %s [ <source_file> ] <destination_file>\n", me);
    exit (1);
  }

  /* Open source file for reading */
  if ( (ifm == NULL) || (strcmp (ifm, "-") == 0) ) {
    sfd = 0;
  } else {
    for (;;) {
      sfd = open (ifm, O_RDONLY, 0);
      if ( (sfd >= 0) || (errno != EINTR) ) break;
    }
    if (sfd < 0) {
      fprintf (stderr, "%s: Cannot open file %s for reading -> %s\n",
               me, ifm, strerror(errno));
      exit (2);
    }
  }

  /* Open dest file for writting (not append, cause we need to lock */
  for (;;) {
    dfd = open (ofm, O_RDWR, 0);
    if ( (dfd >= 0) || (errno != EINTR) ) break;
  }
  if (dfd < 0) {
    fprintf (stderr, "%s: Cannot open file %s for writting -> %s\n",
             me, ofm, strerror(errno));
    Close (sfd);
    exit (3);
  }

  /* Lock dest file */
  for (;;) {
    res = lockf (dfd, F_LOCK, 0);
    if ( (res != 0) || (errno != EINTR) ) break;
  }
  if (res != 0) {
    fprintf (stderr, "%s: Cannot lock file %s for appending -> %s\n",
             me, ofm, strerror(errno));
    (void) Close (sfd);
    (void) close (dfd);
    exit (3);
  }

  /* Prepare dest for append */
  for (;;) {
    offset = lseek (dfd, 0, SEEK_END);
    if ( (offset != (off_t) -1) || (errno != EINTR) ) break;
  }
  if (offset == (off_t) -1) {
    fprintf (stderr, "%s: Cannot seek file %s for appending -> %s\n",
             me, ofm, strerror(errno));
    (void) Close (sfd);
    (void) close (dfd);
    exit (3);
  }

  /* Copy */
  for (;;) {

    /* Read */
    for (;;) {
      rsize = read (sfd, buffer, BLK_SZ);
      if ( (rsize != (size_t) -1) || (errno != EINTR) ) break;
    }
    if (rsize == (size_t) -1) {
      fprintf (stderr, "%s: Cannot read from file %s -> %s\n",
               me, ifm, strerror(errno));
      (void) Close (sfd);
      (void) close (dfd);
      exit (2);
    }

    /* Write */
    for (;;) {
      wsize = write (dfd, buffer, rsize);
      if ( (wsize != (size_t) -1) || (errno != EINTR) ) break;
    }
    if (wsize != rsize) {
      fprintf (stderr, "%s: Cannot write to file %s -> %s\n",
               me, ofm, strerror(errno));
      (void) Close (sfd);
      (void) close (dfd);
      exit (3);
    }

    /* Done when full block could not be read */
    if (rsize != BLK_SZ) {
      break;
    }
  }


  /* Rewind and unlock if possible */
  offset = lseek (dfd, 0, SEEK_SET);
  if (offset != (off_t) -1) {
    (void) lockf (dfd, F_ULOCK, 0);
  }

  /* Close */
  (void) Close (sfd);
  (void) close (dfd);

  /* Done */
  exit (0);

}

