#ifndef DYNLIST_H
#define DYNLIST_H

#include "boolean.h"

/* The list (ADT) */
typedef struct dlist dlist;

/* Initialise a list */
extern void dlist_init (dlist *list, unsigned int data_size);

/* Navigation */
/* Is list empty */
extern boolean dlist_is_empty (dlist *list);
/* List length is 0 or pos_from_first + pos_from_last - 1) */
extern unsigned int dlist_length (dlist *list);
/* Position in list, from start or from end */
/* 0 when list is empty */
/* List length is 0 or pos_from_first + pos_from_last - 1) */
extern unsigned int dlist_get_pos (dlist *list, boolean from_first);
/* Move at beginning or end */
extern void dlist_rewind (dlist *list, boolean at_first);
/* Move one cell forward or backward (if possible) */
extern void dlist_move (dlist *list, boolean forward);

/* Read / insert / delete */
/* Read current (copy from list) */
extern void dlist_read (dlist *list, void * data);
/* Insert after or before current (copy to list and become current) */
extern void dlist_insert (dlist *list, void * data, boolean after_curr);
/* Delete current. Move to next or prev */
extern void dlist_delete (dlist *list, boolean forward);
/* Delete the whole list (re-inits) */
extern void dlist_delete_all (dlist *list);

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

