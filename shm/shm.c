/*
 * Program name: shm.c
 *
 * This program tries to find valid  shared memory addresses to which
 * shared memory segments may be attached.
 * It creates two shared memory segments first and attaches them to
 * the process' virtual memory space. The system determines the shared
 * memory addresses for these segments.
 * Then the program determines the address interval between the two
 * segments. The whole virtual address space is then scanned for valid
 * shared memory addresses with the interval. Scanning may
 * take several minutes.
 *
 * Build the program by:
 *    cc -o shm shm.c
 *
 * This program has an optional command line option : size for a first
 *  shared memory segment to attach to.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>

#include "sig_util.h"

static int shmid1;

static void clean_up (int signum) __attribute__ ((noreturn));
static void clean_up (int signum) {
    if (shmctl(shmid1, IPC_RMID, NULL) < 0) {
        perror("Removing first shm segment");
        exit (1);
    }
    exit ((signum > 0) ? 0 : 1);
}

static unsigned long getul (const char *str, const char *msg) {
    unsigned long result;

    errno = 0;
    if (strncmp(str, "0x", 2) == 0) {
        result = (size_t) strtoull(str, NULL, 16);
    } else {
        result = (size_t) strtoull(str, NULL, 10);
    }
    if (errno != 0) {
        printf ("Invalid argument for %s : %s\n", msg, str);
        exit(1);
    }
    return result;
}

static void usage (void) {
    printf ("Usage: shm [ <basic_size> [ <address> ] ]\n");
    printf ("  By default (no arg) gets first segment of size 1\n");
    printf ("   then following segements of size 1.\n");
    printf ("  With a size, get a segment of this size, either\n");
    printf ("   to OS defined or to provided address.\n");
}


int main(int argc, char *argv[]) {
    char *addr, *addr0, *addr1, *addr2;
    unsigned long tmp_addr;
    int shmid2;
    int interval;
    int valid_shmaddr = 0;
    size_t size0;
    int increasing;
    int ok = 1;

    size0 = 0;
    addr = NULL;
    if (argc == 2) {
      if ( (strcmp (argv[1], "-h") == 0)
        || (strcmp (argv[1], "--help") == 0 ) ) {
        usage();
        exit (1);
      }
    }
    if (argc > 3) {
        printf ("Too many arguments\n");
        usage();
        exit(1);
    } else if (argc >= 2) {
        size0 = getul (argv[1], "<basic_size>");
        if (argc == 3) {
          tmp_addr = getul (argv[2], "<address>");
          addr = (char*) tmp_addr;
        }
    } else {
      addr = 0;
    }
    (void) set_handler (SIGINT, clean_up, (void *) NULL);
    (void) set_handler (SIGTERM, clean_up, (void *) NULL);

    if (size0 != 0) {
        if ((shmid1 = shmget(10, size0, IPC_CREAT | IPC_EXCL | 0777)) < 0) {
            perror("Getting basic shm segment");
            ok = 0;
        }
        if ((addr0 = (char *)shmat(shmid1, addr, 0)) == (char *)-1) {
            perror("Attaching basic shm segment");
            ok = 0;
        }

        if (! ok) {
            exit (1);
        } else {
            printf("Basic shared memory segment (size 0x%lx) at address: %p\n",
                   (unsigned long) size0, addr0);
            printf ("Hit Ctrl C to clean\n");
            while (TRUE) {
                sleep (1);
            }
        }
    }


    if ((shmid1 = shmget(10, 1, IPC_CREAT | IPC_EXCL | 0777)) < 0) {
        perror("Getting first shm segment");
        exit (1);
    }

    if ((shmid2 = shmget(11, 1, IPC_CREAT | IPC_EXCL | 0777)) < 0) {
        perror("Getting second shm segment");
        ok = 0;
    }

    if ((addr1 = (char *)shmat(shmid1, 0, 0)) == (char *)-1) {
        perror("Attaching first shm segment");
        ok = 0;
    }

    if ((addr2 = (char *)shmat(shmid2, 0, 0)) == (char *)-1) {
        perror("Attaching second shm segment");
        ok = 0;
    }

    if (shmdt(addr1) < 0) {
        perror("Detaching first shm segment");
        ok = 0;
    }

    if (shmdt(addr2) < 0) {
        perror("Detaching second shm segment");
        ok = 0;
    }

    if (shmctl(shmid2, IPC_RMID, NULL) < 0) {
        perror("Removing second shm segment");
        ok = 0;
    }

    if (! ok) {
        clean_up(-1);
    } else {
        printf("The system would attach the first shared memory segment at address: %p\n", addr1);
    }

    interval = addr2 - addr1;
    if (interval < 0) {
        /* Address of 2nd shm segment lower than first */
        interval = -interval;
        addr  = addr1;
        addr1 = addr2;
        addr2 = addr;
        increasing = 0;
    } else {
        increasing = 1;
    }

    printf("The shared memory address interval of the system is probably : 0x%X - ", interval);
    if (increasing) {
        printf("Increasing\n");
    } else {
        printf("Decreasing\n");
    }

    printf ("Hit Return for a dump of all addresses ");
    (void) getchar();

    printf("\nThe following valid shared memory addresses were also found:\n");
    for (addr = addr2; addr != addr1; addr = addr + interval) {
        if (addr == (char *)shmat(shmid1, (char *)addr, 0)) {
            printf("%p\n", addr);
            if (shmdt(addr) < 0) {
                perror("Detaching shm");
                ok = 0;
                break;
            }
            valid_shmaddr++;
        }
    }
    if (! ok) {
        clean_up(-1);
    } else {
        printf("\n");
        printf("%d valid shared memory addresses found (first not included)\n", valid_shmaddr);
    }

    clean_up(SIGTERM);
}

