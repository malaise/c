#include <stdio.h>
#include <stdlib.h>

#include "dynlist.h"

int main (void) {

  dlist list;
  int i, v;

  dlist_init (&list, sizeof(int));

  printf ("Add 10 elements\n");
  for (i = 1; i <= 10; i++) {
    dlist_insert (&list, &i, TRUE);
  }

  printf ("Reads 5 elements from the last one:");
  dlist_rewind (&list, FALSE);
  for (i = 1; i <= 5; i++) {
    dlist_read (&list, &v);
    printf (" %02d", v);
    dlist_move (&list, FALSE);
  }
  printf ("\n");

  printf ("List length: %02d\n", dlist_length (&list));

  printf ("Deletes the current\n");
  dlist_delete (&list, TRUE);

  printf ("Pos from first: %02d List length: %02d\n", 
          dlist_get_pos (&list, TRUE), dlist_length (&list));

  printf ("Reads 7 elements from the first one:");
  dlist_rewind (&list, TRUE);
  for (i = 1; i <= 7; i++) {
    dlist_read (&list, &v);
    printf (" %02d", v);
    dlist_move (&list, TRUE);
  }
  printf ("\n");
  dlist_move (&list, FALSE);

  printf ("Adds the element 50 before current position and read: ");
  v = 50;
  dlist_insert (&list, &v, FALSE);
  dlist_read (&list, &v);
  printf ("%02d\n", v);

  printf ("List length: %02d\n", dlist_length (&list));
  printf ("Reads all elements from the first one:");
  dlist_rewind (&list, TRUE);
  for (;;) {
    dlist_read (&list, &v);
    printf (" %02d", v);
    if (dlist_get_pos (&list, FALSE) == 1) break;
    dlist_move (&list, TRUE);
  }
  printf ("\n");


  printf ("Delete all\n");
  dlist_delete_all (&list);
  printf ("List length: %02d\n", dlist_length (&list));

  exit (0);
}

