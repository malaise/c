#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <errno.h>

#include "dynlist.h"

/* Init list */
extern void dlist_init (dlist *list, unsigned int data_size) {
  if (data_size == 0) {
    return;
  }
  list->first = NULL;
  list->last = NULL;
  list->curr = NULL;
  list->pos_first = 0;
  list->pos_last = 0;
  list->data_size = data_size;
}


/* Is list empty */
extern boolean dlist_is_empty (const dlist *list) {
  return (list->curr == NULL);
}

/* List length is 0 or pos_from_first + pos_from_last - 1) */
extern unsigned int dlist_length (const dlist *list) {
  if (list->curr == NULL) {
    return 0;
  } else {
    return list->pos_first + list->pos_last - 1;
  }
}

/* Position in list, from start or from end */
/* 0 when list is empty */
extern unsigned int dlist_get_pos (const dlist *list, boolean from_first) {
  if (from_first) {
    return list->pos_first;
  } else {
    return list->pos_last;
  }
}

/* Move at beginning or end */
extern void dlist_rewind (dlist *list, boolean at_first) {
  if (list->curr == NULL) return;
  if (at_first) {
    list->curr = list->first;
    list->pos_last += list->pos_first - 1;
    list->pos_first = 1;
  } else {
    list->curr = list->last;
    list->pos_first += list->pos_last - 1;
    list->pos_last = 1;
  }
}

/* Move one cell forward or backward (if possible) */
extern void dlist_move (dlist *list, boolean forward) {
  if (list->curr == NULL) return;
  if (forward) {
    if (list->curr->next != NULL) {
      list->curr = list->curr->next;
      list->pos_first++;
      list->pos_last--;
    }
  } else {
    if (list->curr->prev != NULL) {
      list->curr = list->curr->prev;
      list->pos_first--;
      list->pos_last++;
    }
  }
}

/* Read current (copy from list) */
extern void dlist_read (const dlist *list, void * data) {
  if (list->curr == NULL) return;
  memcpy (data, list->curr->data, list->data_size);
}

/* Replace current (insert if list is empty) */
extern void dlist_replace (const dlist *list, void * data) {
  if (list->curr == NULL) return;
  memcpy (list->curr->data, data, list->data_size);
}


/* Adjust prev and next of current to point on current */
/* Or adjust first or last */
static void adjust (dlist *list) {
  if (list->curr->next != NULL) {
    list->curr->next->prev = list->curr;
  } else {
    /* New is last */
    list->last = list->curr;
  }
  if (list->curr->prev != NULL) {
    list->curr->prev->next = list->curr;
  } else {
    /* New is first */
    list->first = list->curr;
  }
}

/* Insert after or before current (copy to list and become current) */
extern void dlist_insert (dlist *list, const void * data, boolean after_curr) {
  cell *ptr, *tmp;

  /* Malloc and copy */
  ptr = malloc (sizeof(cell));
  if (ptr == NULL) {
    perror ("malloc cell");
    return;
  }

  ptr->data = malloc (list->data_size);
  if (ptr->data == NULL) {
    perror ("malloc data");
    return;
  }
  memcpy (ptr->data, data, list->data_size);

  /* Insert */
  if (list->curr == NULL) {
    /* First cell inserted in empty list*/
    list->curr = ptr;
    list->first = list->curr;
    list->last = list->curr;
    list->pos_first = 1;
    list->pos_last = 1;
    list->curr->next = NULL;
    list->curr->prev = NULL;
  } else {
    tmp = list->curr;
    list->curr = ptr;
    if (after_curr) {
      list->curr->prev = tmp;
      list->curr->next = tmp->next;
      list->pos_first++;
    } else {
      list->curr->prev = tmp->prev;
      list->curr->next = tmp;
      list->pos_last++;
    }
    /* Link brothers or first/last to me */
    adjust(list);
  }
}

/* Delete current. Move to next or prev */
extern void dlist_delete (dlist *list, boolean forward) {
  cell *tmp;

  if (list->curr == NULL) return;

  /* Link brothers together */
  tmp = list->curr;
  if (tmp->next != NULL) {
    tmp->next->prev = tmp->prev;
  } else {
    /* Removing last */
    list->last = tmp->prev;
  }
  if (tmp->prev != NULL) {
    tmp->prev->next = tmp->next;
  } else {
    /* Removing first */
    list->first = tmp->next;
  }

  /* Update current and pos */
  if (forward) {
    if (tmp->next != NULL) {
      /* Move forward */
      list->curr = tmp->next;
      list->pos_last--;
    } else {
      /* End of list: try to move backwards */
      forward = FALSE;
    }
  }
  if (!forward) {
    /* Try to move backwards */
    if (tmp->prev != NULL) {
      list->curr = tmp->prev;
      list->pos_first--;
    }
  }

  /* Free data and cell */
  free (tmp->data);
  free (tmp);
}

/* Delete the whole list (re-inits) */
extern void dlist_delete_all (dlist *list) {

  /* Iterate on all from first */
  list->curr = list->first;
  while (list->curr != NULL) {
    list->last = list->curr;
    /* Free data and cell */
    free (list->curr->data);
    free (list->curr);
    /* Next */
    list->curr = list->last->next;
  }

  /* Reset all fields except data size */
  dlist_init (list, list->data_size);
}

/* Local absolute move */
static void move_at (dlist *list, unsigned int offset) {
  unsigned int i;

  dlist_rewind (list, TRUE);
  for (i = 0; i < offset; i++) {
    dlist_move (list, TRUE);
  }
}

/* Local swap of 2 cells */
static void swap (dlist *list, cell *left, cell *right) {

  cell *next, *prev;

  prev = left->prev;
  next = left->next;

  if ( (left->next != right) && (left->prev != right) ) {
    /* Not adjacent cells */
    /* Exchange neighbours links */
    if (left->prev != NULL) {
      left->prev->next = right;
    } else {
      list->first = right;
    }
    if (left->next != NULL) {
      left->next->prev = right;
    } else {
      list->last = right;
    }
    if (right->prev != NULL) {
      right->prev->next = left;
    } else {
      list->first = left;
    }
    if (right->next != NULL) {
      right->next->prev = left;
    } else {
      list->last = left;
    }

    /* Exchange swapped cells links to neighbours */
    left->prev = right->prev;
    left->next = right->next;
    right->prev = prev;
    right->next = next;

  } else if (left->next == right) {
    /* Left just before right */
    /* Exchange neighbours links */
    if (left->prev != NULL) {
      left->prev->next = right;
    } else {
      list->first = right;
    }
    if (right->next != NULL) {
      right->next->prev = left;
    } else {
      list->last = left;
    }

    /* Exchange swapped cells links to neighbours */
    left->prev = right;
    left->next = right->next;
    right->prev = prev;
    right->next = left;
  } else if (left->prev == right) {
    /* Left just after right */
    /* Exchange neighbours links */
    if (left->next != NULL) {
      left->next->prev = right;
    } else {
      list->last = right;
    }
    if (right->prev != NULL) {
      right->prev->next = left;
    } else {
      list->first = left;
    }

    /* Exchange swapped cells links to neighbours */
    left->prev = right->prev;
    left->next = right;
    right->prev = left;
    right->next = next;
  }

}


/* Local recursive sorting */
static void subqsort (dlist *list,
                      const unsigned int left, const unsigned int right,
                      less_than_func *less_than) {
  /* Middle of the slice */
  const unsigned int i_front = (left + right ) / 2;
  cell *l_front;
  /* Both halfs of the slice */
  unsigned int i_left, i_right;
  cell *l_left, *l_right;

  i_left = left;
  i_right = right;
  /* Set link to frontier */
  move_at (list, i_front);
  l_front = list->curr;

  for (;;) {
    /* First element at left of slice and not positioned ok */
    /*  regarding the frontier */
    move_at (list, i_left - 1);
    while (less_than (list->curr->data, l_front->data)) {
      dlist_move (list, TRUE);
    }
    l_left = list->curr;
    i_left = dlist_get_pos (list, TRUE);

    /* Last  element a right of slice and not positioned ok */
    /*  regarding the frontier */
    move_at (list, i_right - 1);
    while (less_than (l_front->data, list->curr->data)) {
      dlist_move (list, FALSE);
    }
    l_right = list->curr;
    i_right = dlist_get_pos (list, TRUE);

    /* Exchange and go to next elements if not both in frontier */
    if (i_left < i_right) {
      swap (list, l_left, l_right);
      i_left ++;
      i_right --;
    } else if (i_left == i_right) {
      /* Go to next elements if not crossed */
      if (i_left != right) {
        i_left ++;
      }
      if (i_right != left) {
        i_right --;
      }
    }

    /* Leave if crossed now */
    if (i_left > i_right) break;
    if ( (i_left == right) && (i_right == left) ) break;
  }

  /* Sort both new slices */
  if (left   < i_right) subqsort(list, left,   i_right, less_than);
  if (i_left < right)   subqsort(list, i_left, right, less_than);

}

/* Quick sort the list according to comp function */
extern void dlist_sort (dlist *list, less_than_func *less_than) {

  unsigned int last = dlist_length (list);

  if (last <= 1) {
    return;
  }
  subqsort (list, 1, last, less_than);
  dlist_rewind (list, TRUE);
}

