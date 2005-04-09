#include <stdio.h>
#include "module1.h"

extern void __init_11(void);
extern void __init_12(void);
extern void _init(void);

extern void __init_11(void) {
  printf ("Init 11\n");
}

extern void __init_12(void) {
  printf ("Init 12\n");
}

extern void _init(void) {
  printf ("Init 1\n");
}

extern void func1(void) {
  printf ("Func 1\n");
}

