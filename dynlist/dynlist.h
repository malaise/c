#ifndef DYNLIST_H
#define DYNLIST_H

#include "boolean.h"

/* The list (ADT) */
typedef struct dlist dlist;

/* Initialise a list */
extern void dlist_init (dlist *list, unsigned int data_size);

/* NAVIGATION */
/* Is list empty */
extern boolean dlist_is_empty (const dlist *list);
/* List length is 0 or pos_from_first + pos_from_last - 1) */
extern unsigned int dlist_length (const dlist *list);
/* Position in list, from start or from end */
/* 0 when list is empty */
/* List length is 0 or pos_from_first + pos_from_last - 1) */
extern unsigned int dlist_get_pos (const dlist *list, boolean from_first);
/* Move at beginning or end */
extern void dlist_rewind (dlist *list, boolean at_first);
/* Move one cell forward or backward (if possible) */
extern void dlist_move (dlist *list, boolean forward);

/* READ / INSERT / DELETE */
/* Read current (copy from list) */
extern void dlist_read (const dlist *list, void * data);
/* Replace current */
extern void dlist_replace (const dlist *list, void * data);
/* Insert after or before current (copy to list and become current) */
extern void dlist_insert (dlist *list, const void * data, boolean after_curr);
/* Delete current. Move to next or prev */
extern void dlist_delete (dlist *list, boolean forward);
/* Delete the whole list (re-inits) */
extern void dlist_delete_all (dlist *list);

/* SORT */
/* Type of function that compares 2 data. */
/* Must be strict: less_than (p, p) -> FALSE */
typedef boolean less_than_func (const void *, const void*);
/* Quick sort the list according to less_than function and rewind */
extern void dlist_sort (dlist *list, less_than_func *less_than);

/* Private */
struct cell;

typedef struct cell {
  struct cell * prev;
  struct cell * next;
  void * data;
} cell;

struct dlist {
  cell * first;
  cell * last;
  cell * curr;
  unsigned int pos_first;
  unsigned int pos_last;
  unsigned int data_size;
};

#endif

