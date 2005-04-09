#ifndef __CIRCUL_H
#define __CIRCUL_H

#include <stdio.h>

struct cir_file;

extern char CIR_EOF;

/* This routine attemps to open a \a size byte circular file \a
 * filename. The \a mode parameter is the same as in \c fopen(3).
 * \return a pointer to an allocated \c cir_file structure or NULL if an error
 * occured.
 * \warning This routine performs dynamic memory allocation : don't forget to
 * call \c cir_close when finished.
 */
extern struct cir_file* cir_open(const char* filename, const char* mode, unsigned int size);

/* This routine closes the circular file \a fd and free the \c cir_file
 * structure.
 * \return 0 or -1 if an error occured.
 */
extern int cir_close(struct cir_file* fd);

/* This routine attempts to write to the circular file \a fd, much like
 * the write(2) system call.
 * \return the number of written bytes or -1 if an error occured.
 */
extern int cir_write(struct cir_file* fd, char* buffer, unsigned int size);

/* This routine attempts to read from the circular file \a fd, much like
 * the read(2) system call.
 * \return the number of read bytes or -1 if an error occured.
 */
extern int cir_read(struct cir_file* fd, char* buffer, unsigned int size);

/* This routine attempts to read a string from the circular file \a fd,
 * much like the gets(3) standard C library call.
 * \return the length of the read string or -1 if an error occured.
 */
extern int cir_gets(struct cir_file* fd, char* buffer, unsigned int size);

#endif 

