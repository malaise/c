#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vt100.h"

void clrscr (void) {
  char chaine[5];

  strcpy(chaine," [2J");
  chaine[0]=27;
  printf("%s",chaine);
  gotoxy (0, 0);
}

void gotoxy (int x, int y) {
  char chaine2[15];

  strcpy(chaine2," [  ;  H");
  chaine2[0]=27;
  chaine2[2]='0'+x/10;
  chaine2[3]='0'+x%10;
  chaine2[5]='0'+y/10;
  chaine2[6]='0'+y%10;
  printf("%s",chaine2);
}

void open_keybd (void) {
  system("stty -echo raw");
}

void close_keybd (void) {
  system("stty echo -raw");
}

#define cprintf printf

void highvideo (void) {
  char chaine[9];

  strcpy(chaine ," [1m");
  chaine[0]=27;
  printf("%s",chaine);
}

void lowvideo (void) {
  char chaine[9];

  strcpy(chaine ," [0m");
  chaine[0]=27;
  printf("%s",chaine);
}

long filelength (int fd) {
  struct stat buf;

  fstat(fd, &buf);
  return ((long) buf.st_size);
}



static char escape_sequence (void) {
  int car;

  car = getchar();

  switch (car) {
    case 0x5b:
      /* Esc [ */
      car = getchar();
      switch (car) {
        case 'A':    /* Arrow up    */
          return 1;  /* Arrow up    */
        case 'C':    /* Arrow right */
          return 2;  /* Arrow right */
        case 'B':    /* Arrow down  */
          return 3;  /* Arrow down  */
        case 'D':    /* Arrow left  */
          return 4;  /* Arrow left  */
        default :
          return 0;
    }
    case 'A':        /* Esc A       */
    case 'a':        /* Esc a       */
      return 20;     /* Begin file  */
    case 'Z':        /* Esc Z       */
    case 'z':        /* Esc z       */
      return 19;     /* End file    */
    case 'H':        /* Esc H       */
    case 'h':        /* Esc h       */
      return 10;     /* Home        */
    case 'E':        /* Esc E       */
    case 'e':        /* Esc e       */
      return 11;     /* End         */
    case 'F':        /* Esc F       */
    case 'f':        /* Esc f       */
      return 15;     /* Find        */
    case 'P':        /* Esc P       */
    case 'p':        /* Esc p       */
      return 16;     /* Goto page   */
    case 'S':        /* Esc S       */
    case 's':        /* Esc s       */
      return 17;     /* Write       */
    case 'U':        /* Esc U       */
    case 'u':        /* Esc u       */
      return 23;     /* Write       */
    case 'X':        /* Esc X       */
    case 'x':        /* Esc x       */
      return 18;     /* Exit        */
    case 'C':        /* Esc C       */
    case 'c':        /* Esc c       */
      return 21;     /* Exit        */
    case 'Q':        /* Esc C       */
    case 'q':        /* Esc c       */
      return 22;     /* Exit        */
    case 0x1B:       /* Escape      */
      return 0x1B;   /* New escape  */
    default :
      return 0;
   }
}

char read_char (void) {
  int car;

  for (;;) {
    car = getchar();
    if ( (car > 0x1F) && (car < 0x7F) ) return car;
    switch (car) {
      case 0x7F:     /* Backspace  */
      case 0x08:     /* Backspace  */
        return 8;    /* Backspace  */
      case 0x10:     /* Ctrl P     */
        return 5;    /* Pg up      */
      case 0x0E:     /* Ctrl N     */
        return 6;    /* Pg down    */
      case 0x09:     /* Tab        */
        return 9;    /* Tab        */
      case 0x0D:     /* Return     */
        return 13;   /* Return     */
      case 0x1B:     /* Esc        */
        while (1) {
          car = escape_sequence();
          if (car == 0x00) break;
          else if (car != 0x1B) return car;
        }
    }
  }
}

void beep (unsigned char nb_beeps,
           unsigned int frequency __attribute__ ((unused))) {
  int i;

  for (i = 1; i <= nb_beeps; i++) {
    putchar ((int) 0x07);

  }
}

