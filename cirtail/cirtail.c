/*
 SPACF    DATE      CORRECTOR         DESCRIPTION
          311096    P.MALAISE         Creation
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/time.h>

#include "circul.h"
  
#define DEFAULT_DELAY_MS 1000

#define BUFFER_SIZE 512


int main (int argc, char *argv[]) {

char c;
FILE * circul_file;
int delay_ms;
char * circular_file_name;
struct timeval delay;
long last_pos;
char buffer [BUFFER_SIZE];
int len;
long nb_block;
int offset;

    circular_file_name = NULL;
    delay_ms = DEFAULT_DELAY_MS;
    if (argc == 2) {
        delay_ms = DEFAULT_DELAY_MS;
        circular_file_name = argv[1];
    } else {
        if ( (argc == 4) && (strcmp(argv[1], "-i") == 0) ) {
            delay_ms = atoi (argv[2]);
            if (delay_ms > 0) {
                circular_file_name = argv[3];
            }
        }
    } 

    if (circular_file_name == NULL) {
        fprintf (stderr, "Syntax error. Syntax %s [ -i <delay_ms> ] <circular_file_name>\n", argv[0]);
        exit (1);
    } 

    /* Open linear file */
    circul_file = fopen(circular_file_name, "r");
    if (circul_file == (FILE*) NULL) {
        perror ("fopen");
        fprintf(stderr, "Cannot open the circular file %s\n", circular_file_name);
        exit (1);
    } 
  
    /* Locate mark */
    /* Look first end of file */
    (void) fseek(circul_file, -1, SEEK_END);
    c = getc(circul_file);

    if (c !=  CIR_EOF) {
        /* Look for the mark from beginning */
        (void) fseek(circul_file, 0, SEEK_SET);
        c = '\0';
        nb_block = 1;
        do {
            len = fread (buffer, 1, BUFFER_SIZE, circul_file);
            if (len == 0) {
                /* Mark has moved back at the beginning during our search */
                /* Give up the search and let's go to standard behaviour */
                nb_block = 1;
                offset = 0;
                c = CIR_EOF;
            } else {
                for (offset = 0; offset < len; offset++) {
                    if (buffer[offset] == CIR_EOF) {
                        c = CIR_EOF;
                        break;
                    }
                }
                nb_block ++;
            }
        } while (c != CIR_EOF);
        (void) fseek(circul_file, offset - len, SEEK_CUR);
    } else {
        /* Mark is at end of file */
        fseek (circul_file, -1, SEEK_CUR);
        nb_block = -1;
    }

    /* Mark is found */
    /* Go back one block to have something to display */
    if (nb_block == 1) {
        (void) fseek(circul_file, 0, SEEK_SET);
    } else {
        (void) fseek(circul_file, -BUFFER_SIZE, SEEK_CUR);
    }

    for (;;) { 
        c = getc(circul_file);  

        if ( (c != CIR_EOF) && (c != EOF) ) {
            /* Normal behaviour */
            (void) putchar (c);
        } else {
            if (c == EOF) {
                /* End of file. Look for mark from beginning of file */
                last_pos = 0;
                fclose (circul_file);
            } else {
                /* Mark found. Current end of file */
                /* Close file, wait a bit, then look for mark from same location */
                last_pos = ftell (circul_file) - 1;
                fclose (circul_file);
                (void) fflush (stdout);
                delay.tv_sec = delay_ms / 1000;
                delay.tv_usec = (delay_ms % 1000) * 1000;
                (void) select (0, (fd_set *) NULL, (fd_set *) NULL, (fd_set *) NULL, &delay);
            }
            circul_file = fopen(circular_file_name, "r");
            if (circul_file == (FILE*) NULL) {
                perror ("fopen");
                fprintf(stderr, "Cannot re-open the circular file %s\n", circular_file_name);
                break;
            } 
            (void) fseek(circul_file, last_pos, SEEK_SET);
        }
    }
    exit(1);
}

