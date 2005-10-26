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
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

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


int main(int argc, char *argv[]) {
    char *addr, *addr0, *addr1, *addr2;
    int shmid0, shmid2;
    int interval;
    int valid_shmaddr = 0;
    int size0;
    int increasing;
    int ok = 1;

    if (argc > 2) {
        printf ("Too many arguments. Supported is [ <basic_size> ]\n");
        exit(1);
    } else if (argc == 2) {
        if (strncmp(argv[1], "0x", 2) == 0) {
            size0 = (int) strtol(argv[1], NULL, 16);
        } else {
            size0 = (int) strtol(argv[1], NULL, 10);
        }
        if (size0 == 0) {
            printf ("Invalide argument for <basic_size> : %s\n", argv[1]);
            exit(1);
        }
    } else {
        size0 = 0;
    }

    if (size0 != 0) {
        if ((shmid0 = shmget(10, size0, IPC_CREAT | IPC_EXCL | 0777)) < 0) {
            perror("Getting basic shm segment");
            ok = 0;
        }
        if ((addr0 = (char *)shmat(shmid0, 0, 0)) == (char *)-1) {
            perror("Attaching basic shm segment");
            ok = 0;
        }
        if (shmdt(addr0) < 0) {
            perror("Detaching basic shm segment");
            ok = 0;
        }

        if (shmctl(shmid0, IPC_RMID, NULL) < 0) {
            perror("Removing basic shm segment");
            ok = 0;
        }
        if (! ok) {
            exit (1);
        } else {
            printf("Basic shared memory segment (size 0x%X) at address: 0x%p\n\n", size0, addr0);
        }
        exit (0);
    }

    (void) set_handler (SIGINT, clean_up, (void *) NULL);
    (void) set_handler (SIGTERM, clean_up, (void *) NULL);

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
        printf("The system would attach the first shared memory segment at address: 0x%p\n", addr1);
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

    printf ("Hit a Return ");
    (void) getchar();

    printf("\nThe following valid shared memory addresses were also found:\n");
    for (addr = addr2; addr != addr1; addr = addr + interval) {
        if (addr == (char *)shmat(shmid1, (char *)addr, 0)) {
            printf("0x%p\n", addr);
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
