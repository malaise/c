/* Includes
 ******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#include "circul.h"

char CIR_EOF = 0X1; /* ^A */

typedef enum { READ, WRITE, WRITE_APPEND } TYPE_FILE_CIRCULAR;
typedef enum { IS_FOUND, IS_NOT_FOUND }  MARK_FOUND;
typedef MARK_FOUND    END_FOUND;

struct cir_file
{
    FILE          *ptf;       /* File descriptor of circular file */
    unsigned int  size_max;   /* size (octet) of this file   */
    TYPE_FILE_CIRCULAR mode;  /* open mode */
    MARK_FOUND    mark;       /* CIR_EOF already search */
    END_FOUND     end;        /* CIR_EOF already found */
};


/* Routines
 ******************************************************************************/
static int recherche_marque (struct cir_file * fd);
static int write_mfb (struct cir_file * fd);
static int local_fwrite (char *buff ,int size ,unsigned int lg ,FILE *fd );

/******************************************************************************/
int cir_close (struct cir_file *fd)
{
    int cr = 0;

    if (!fd) return -1;

    fd->mark= IS_NOT_FOUND;
    fd->end = IS_NOT_FOUND;
    cr = fclose (fd->ptf);

    free(fd);
    return (cr == EOF ? -1 : 0);
}

/******************************************************************************/
int cir_gets (struct cir_file *fd, char *buffer,  unsigned int lg)
{
    int lu = 0;
    char c = EOF;
    union {
        char *ptc;
        void *ptv;
    } X;

    if (fd == (struct cir_file *) NULL)  return -1;

    if (fd->mode != READ) return -1;

    if (lg == (unsigned int) 0) return 0;

    /* la fin a deja ete rencontree a la lecture precedente */
    if (fd->end == IS_FOUND) return 0;

    if (fd->mark == IS_NOT_FOUND) {
        /* la marque n'a pas ete trouvee */
        lu = recherche_marque (fd);
        if (lu == 0) return -1;
    }

    X.ptc = buffer;

    /* Reads characters from fd into buffer, until either a newline character
     * is read, byte_count - 1 characters have been read, or CIR_EOF
     * is seen.
     * Finishes by appending a null character and returning buffer.
     * If CIR_EOF is seen before any characters have been written,
     * the function returns -1 without appending the null character.
     * If there is a file error, always return -1.
     */

    while (--lg > 0 && (c = getc(fd->ptf)) != CIR_EOF) {
        if (c == EOF) {
            if ( fseek (fd->ptf, 0, SEEK_SET)== -1 ) return -1;
            lg++;
        }
        else if ((*(X.ptc)++ = c) == '\n') {
            break;
        }
    }

    *(X.ptc) = '\0';

    if (c == CIR_EOF) {
        fd->end = IS_FOUND;
        return 0;
     }

    return strlen(buffer);
}

/******************************************************************************/
struct cir_file* cir_open(const char *path, const char *mode, unsigned int lg)
{
    struct cir_file* cir_fp;
    TYPE_FILE_CIRCULAR m;
    FILE*     fp;
    char     file_mode[3];

    if (!mode) return NULL;

    switch (tolower(mode[0])) {
        case 'r': strcpy(file_mode, "r");  m = READ;  break;
        case 'w': strcpy(file_mode, "w+"); m = WRITE; break;
        default: return NULL;
    }

    if ( (fp = fopen(path, file_mode)) == NULL) return NULL;

    cir_fp = calloc(1, sizeof(struct cir_file));
    cir_fp->ptf  = fp;
    cir_fp->mode = m;
    cir_fp->mark = IS_NOT_FOUND;
    cir_fp->end  = IS_NOT_FOUND;
    cir_fp->size_max   = lg;

    return cir_fp;
}

/******************************************************************************/
static int recherche_marque (struct cir_file *fd)
{
    union {
        char *ptc;
        void *ptv;
    } X;
    char buf[1];
    register int i;

    X.ptc=buf;
    fd->mark = IS_FOUND;
    fd->end = IS_NOT_FOUND;

    i = ftell (fd->ptf);

    do {
        do {
            i = fread (X.ptv, 1, 1, fd->ptf);
        } while ( (i== -1) && ( errno==EINTR));
    } while ((i != 0) && (buf[0] != (char) CIR_EOF));
    return (i);
}

/******************************************************************************/
int cir_read (struct cir_file *fd, char* buffer, unsigned int size)
{
    int lu = 0, i = 0;
    union {
        char *ptc;
        void *ptv;
    } X;


    if (fd == NULL)  return -1;
    if (fd->mode != READ) return -1;
    if (size == 0)  return 0;

    /* la fin a deja ete rencontree a la lecture precedente */
    if (fd->end == IS_FOUND) return 0;

    if (fd->mark == IS_NOT_FOUND) {    /* la marque n'a pas ete trouvee */
        if ( (lu = recherche_marque(fd)) == 0) return -1;
    }

    X.ptc=buffer;

    do {
        lu = fread (X.ptv, 1, size, fd->ptf);
    } while ( (lu==-1)&&(errno==EINTR));

    if (lu == 0) {
        if ( fseek (fd->ptf, 0, SEEK_SET)== -1 ) return -1;
        do {
            lu = fread (X.ptv, 1, size, fd->ptf);
        } while ( (lu==-1)&&(errno==EINTR));
    }

    for (i = 0; i < lu; i++) {
        if (X.ptc[i] == (char) CIR_EOF) {
            fd->end = IS_FOUND;
            return i;
        }
    }

    return lu;
}

/*
   ERROR MANAGEMENT
   if return = 0 no error
   if return > 0 impossible
   if return < 0 then val give the error

   possible error -
   1 : the file is open in read mode
   2 : error on write in the circular file
   3 : error on moving in the circular file
   4 : error on write in the circular file
   5 : error on moving in the circular file
   6 : error on moving in the circular file
 */

/******************************************************************************/
int cir_write (struct cir_file *fd, char *buffer, unsigned int lg)
{
    long position;
    int cr = 0;
    union {
        char *ptc;
        void *ptv;
    } X;

    if (fd == (struct cir_file *) NULL) {
        return -1;
    }

    if (fd->mode != WRITE)  {
        return -1;
    }

    if (lg == (unsigned int) 0) {
        return (write_mfb (fd));
    }

    X.ptc = buffer;
    position = ftell (fd->ptf);

    if (position + lg + 1 > fd->size_max) {
        register unsigned int rest_file = fd->size_max - position;
        register unsigned int rest_buffer = lg - rest_file;

        if (fseek (fd->ptf, 0, SEEK_SET) == -1) {
            return -1;
        }
        if (write_mfb (fd) == -1) {
            return -1;
        }
        if (fseek (fd->ptf, position, SEEK_SET) == -1) {
            return -1;
        }

        cr = local_fwrite (X.ptc, 1, rest_file, fd->ptf);

        if ((unsigned int)cr != rest_file) {
            return -1;
        }

        if (fseek (fd->ptf, 0, SEEK_SET) == -1) {
            return -1;
        }

        return (cir_write (fd, X.ptc + rest_file, rest_buffer));

    } else {
        /* position + lg+1 <= fd->size_max */
        if (fseek (fd->ptf, position + lg, SEEK_SET) == -1) {
            return -1;
        }

        if (write_mfb (fd) == -1) {
            return -1;
        }

        if (fseek (fd->ptf, position, SEEK_SET) == -1) {
            return -1;
        }

        cr = local_fwrite (X.ptc, 1, lg, fd->ptf);

        if ((unsigned int)cr != lg) {
            return -1;
        }
    }
    return (write_mfb (fd));
}

/******************************************************************************/
static int write_mfb (struct cir_file *fd)
{
    union {
        char *ptc;
        void *ptv;
    } X;
    int cr;

    X.ptc = &CIR_EOF;

    if (ftell (fd->ptf) + 1 > (long)fd->size_max) {
        if (fseek (fd->ptf, 0, SEEK_SET) == -1) {
            return -1;
        }
    }

    do {
        cr=fwrite (X.ptv, 1, 1, fd->ptf);
    } while ( (cr== -1) &&(errno==EINTR));

    if ( cr !=  1 ) {
        return -1;
    }

    if (fseek (fd->ptf, -1, SEEK_CUR) == -1) {
        return -1;
    }
    return 0;
}

/******************************************************************************/
static int local_fwrite (char *buff ,int size ,unsigned int lg ,FILE *fd )
{
    union {
        char *ptc;
        void *ptv;
    } X;
    register int lg_to_write = lg ;
    register int lg_written  = 0 ;
    register int cr    = 0 ;

    X.ptc = buff;

    do {

        X.ptc = X.ptc + lg_written;

        do {
            cr = fwrite (X.ptv, (size_t)size, (size_t)lg_to_write, fd);
        } while ( (cr== -1) &&(errno==EINTR));
        if ( cr == -1 ) return (-1);
        lg_to_write -= cr ;
        lg_written  += cr ;

    } while ( lg_to_write != 0 ) ;

    return (lg_written);
}

