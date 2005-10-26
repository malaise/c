#include <stdio.h>
#include <stdlib.h>
#include <locale.h>

#define ALL 1
#define ONE 0

static void draw (char *str[], int  count, char param);
static void help(char *str[]);

int main (int argc, char *argv[]) {

  (void) setlocale(LC_ALL, "");

  if ( argc < 2 ) help(argv);
  else draw (argv, argc, ONE);
  exit(0);
}

static void help (char *str[]) {
	printf ("-- usage : \n");
	printf ("           ascii [ARG]\n");
	printf ("             where ARG is a character or a string\n");
	printf ("             if no argument is specified \n");
	printf ("             print the whole ascii table\n\n");
        draw (str, ALL, ' ');
}

static void draw (char *str[], int  count, char param)
                 __attribute__ ((noreturn));
static void draw (char *str[], int  count, char param) {
  int j=0, i=0;

  printf ("+-----------+-----+-----+-----+\n");
  printf ("| character | dec | oct | hex |\n");
  printf ("+-----------+-----+-----+-----+\n");

  if ( param == ONE )
  {
     for ( j = 1; j < count; )
     {
        i = 0;
        while ( str[j][i] != '\0' )
        {
           printf ("|     %c     | %3d | %3o | %3X |\n", str[j][i], str[j][i], str[j][i], str[j][i]);
           i++;
         }
         if ( j < count - 1) printf ("+-----------+-----+-----+-----+\n");
         j++;
     }
  }
  else
  {
     for ( i = 32; i < 256; i++)
     {
        printf ("|     %c     | %3d | %3o | %3X |\n", i, i, i, i);
     }
  }

  printf ("+-----------+-----+-----+-----+\n");

  exit ( 0 );

}

