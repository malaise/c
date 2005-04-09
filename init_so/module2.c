#include <stdio.h>
#include "module2.h"

extern void __init_21(void);
extern void __init_22(void);
extern void _init(void);

extern void __init_21(void) {
  printf ("Init 21\n");
}

extern void __init_22(void) {
  printf ("Init 22\n");
}

extern void _init(void) {
  printf ("Init 2\n");
}

extern void func2(void) {
  printf ("Func 2\n");
}

