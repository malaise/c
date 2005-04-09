#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <signal.h>

static char *b0;
static char *p;
static int size_handler;

static void sig_handler (int sig_num) {


  if (p == NULL) {
      size_handler=rand()%1000+1;
      p = malloc (size_handler);
  }

}


  

int main(void) {
  int size;

  (void) signal (SIGUSR1, sig_handler);

  p=NULL;

   for (;;) {
     size = rand() %1000+1;
     b0 = malloc (size);
     if (p != NULL) {
         if ( (p >= b0) && (p <= b0 + size) ) {
             printf ("Fatal! %p %p %p\n" ,b0, p, b0 + size);
             printf ("Sizes %d %d\n", size, size_handler);
             exit (1);
         }
         if ( (b0 >= p) && (b0 <= p + size_handler) ) {
             printf ("Fatal!! %p %p %p\n" ,b0, p, b0 + size);
             printf ("Sizes %d %d\n", size, size_handler);
             exit (1);
         }
         free (p);
         p = NULL;
      }
      free (b0);
   }
   exit(0);
}


     

