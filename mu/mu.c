/*----------------------------------------------------------------------------*/
/* Ver | Date     | Who        | What                                         */
/*----------------------------------------------------------------------------*/
/* 2.4 | 19980701 | P. Malaise | Manage input key 7f as 08 (dos.c)            */
/*     |          |            | If EscA in search, no reset of length,       */
/*     |          |            |  prev string is kept                         */
/* 2.5 | 19980818 | P. Malaise | Display offset                               */
/* 2.6 | 19990310 | P. Malaise | Port to AIX (MU_PAGE_SIZE)                   */
/* 2.7 | 19990325 | P. Malaise | Bold->norm does not erase all the characater */
/* 2.8 | 19990711 | P. Malaise | Main returns int                             */
/* 2.9 | 20000127 | P. Malaise | Fix many warnings                            */
/* 2.A | 20110329 | P. Malaise | Add getenv of unprintable character          */
/* 2.B | 20120726 | P. Malaise | Support file length larger than 31 bits      */
/* 3.0 | 20230706 | P. Malaise | Add PgUp/Down and Home/End                   */
/*----------------------------------------------------------------------------*/
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include "vt100.h"

#define TITLE "Malaise utilities - V3.0 -->"
#define USAGE "Usage : mu [-r] file\n"

#define ABORT(str) (printf ("%s\n", str), exit(1))

#define MU_PAGE_SIZE 256

typedef enum {true=1, false=0} boolean;
typedef unsigned char byte;

/* Char for unprintable code */
char default_unprintable='~';
char *unprintable=&default_unprintable;

/* File name and descriptor */
static char *file_name;
static int  file_des;

/* Number of bytes of the file */
static long long file_size;

/* Last page no 1 .. */
static unsigned int last_page;
/* Current page no 1 .. last_page */
static unsigned int current_page;

/* Last of last and current page */
static unsigned short last_page_size;
static unsigned short current_page_size;

/* Current copy of page */
static byte tab[MU_PAGE_SIZE];

/* Current pos in page */
static unsigned short pos;

/* First or second hexa digit */
static boolean first_hexa;

/* In hexa or ascii side */
static boolean hexa_side;

/* Current page modified */
static boolean modified;

/* Message to erased at next input */
static boolean to_erase;

/* Can mu write on file (can modify) */
static boolean can_modify;

/* SCREEN ****************************************************************/

static void mvprint (int x, int y, const char chaine1[]) {
  gotoxy (x, y);
  cprintf ("%s", chaine1);
}

/* Unused
 * static void mvprints (int x, int y, short num) {
 *   char str[9];
 *   short l = 0;
 *
 *   if (num < 0)   {str[l ++] = '-'; num = - num;}
 *   if (num > 9999) str[l ++] = (num / 10000) + 48;
 *   if (num > 999)  str[l ++] = ((num % 10000)/ 1000) + 48;
 *   if (num > 99)   str[l ++] = ((num % 1000)/ 100) + 48;
 *   if (num > 9)    str[l ++] = ((num % 100) / 10) + 48;
 *   str[l ++] = (num % 10) + 48;
 *
 *   str[l] = '\0';
 *   mvprint (x, y, str);
 * }
 */

/* TITLE *****************************************************************/

#define FILE_NAME_LEN 25
#define PREFIX "... "
static void title (void) {
  char printed_file_name [FILE_NAME_LEN];
  char *p;

  if (strlen(file_name) < FILE_NAME_LEN) {
    strcpy (printed_file_name, file_name);
  } else {
    p = file_name + strlen(file_name) - (FILE_NAME_LEN-1);
    strcpy (printed_file_name, p);
    memcpy (printed_file_name, PREFIX, strlen(PREFIX));
  }

  mvprint (1,  9,"Page");
  mvprint (1, 25, TITLE);
  if (!can_modify) {
    highvideo ();
  }
  mvprint (1, 55, printed_file_name);
  lowvideo();
  mvprint (2, 25, "Esc -> Find Page Save Undo eXit Quit Home End A Z");
  mvprint (3, 25, "Tab Arrows Home/End PgUp/PgDown Ctrl+Nextpage/Prevpage");
}



static void usage (void) {

  printf (USAGE);
}


/* FILE ******************************************************************/

static long file_pos (unsigned int page, unsigned short pos) {
  return (MU_PAGE_SIZE * ((long) page - 1L) + pos);
}

static void read_file (void) {

  lseek (file_des, file_pos (current_page, 0), SEEK_SET);

  read (file_des, tab, current_page_size);

}

static void write_file (void) {
  lseek (file_des, file_pos (current_page, 0), SEEK_SET);

  write (file_des, tab, current_page_size);

}

static void open_file (void) {

  file_des = open (file_name, O_RDWR);
  if (file_des == -1) {
    can_modify = false;
    file_des = open (file_name, O_RDONLY);
    if (file_des == -1) {
      printf ("Can't open file \"%s\"\n", file_name);
      perror ("open");
      usage ();
      exit (0);
    }
  }

  file_size = filelength (file_des);
  if (file_size == 0) {
    printf ("Empty file \"%s\"\n", file_name);
    exit (0);
  }

  last_page      = (file_size-1) / MU_PAGE_SIZE + 1;
  last_page_size = (file_size-1) % MU_PAGE_SIZE + 1;

}

static void write_modif (void);
static void display_page (void);
static boolean confirm (char yes, char no);

/* MOVEMENT **************************************************************/

static void new_page (void) {

  if (current_page != last_page)
    current_page_size = MU_PAGE_SIZE;
  else
    current_page_size = last_page_size;
}


static void page_down (void) {

  write_modif ();

  if (current_page < last_page)
    current_page ++;
  else
    current_page = 1;

  new_page ();
  read_file ();
  display_page ();
  if (pos >= current_page_size) pos = current_page_size -1;
}

static void page_up (void) {

  write_modif ();

  if (current_page > 1)
    current_page --;
  else
    current_page = last_page;

  new_page ();
  read_file ();
  display_page ();
  if (pos >= current_page_size) pos = current_page_size -1;
}

static void g_first_page (void) {

  write_modif ();

  current_page = 1;

  new_page ();
  read_file ();
  display_page ();

  pos = 0;
}

static void g_last_page (void) {

  write_modif ();

  current_page = last_page;

  new_page ();
  read_file ();
  display_page ();

  pos = 0;
}


static void movement (short dx, short dy) {

  first_hexa = true;

  if ((short)pos + dy + 16 * dx >= MU_PAGE_SIZE) {
    pos = ((short)pos + dy + 16 * dx) % MU_PAGE_SIZE;
    page_down ();
  } else if ((short)pos + dy + 16 * dx < 0) {
    pos = ((short)pos + dy + 16 * dx + MU_PAGE_SIZE) % MU_PAGE_SIZE;
    page_up ();
  } else if ((short)pos + dy + 16 * dx >= current_page_size) {
    /* End of file */
    if (dy == 1) {
      pos = 0;
    } else {
      pos = pos % 16;
    }
    page_down ();
  } else {
    pos = (short)pos + dy + 16 * dx;
  }
}

static void home_mvt (void) {

  pos = 0;
}

static void end_mvt (void) {

  pos = current_page_size - 1;
}
/* PRINT PAGE ************************************************************/

static void move_to_hexa (unsigned short p, boolean first_digit) {
  int x, y;

  x = p / 16 + 5;
  y = (p % 16) * 3 + 5 + (p % 16 > 7 ? 3 : 0);
  if (! first_digit) {
    y ++;
  }
  gotoxy (x, y);
}

static void move_to_ascii (unsigned short p) {
  int x, y;

  x = p / 16 + 5;
  y = p % 16 + 60;
  gotoxy (x, y);
}

static void move_to (unsigned short p, boolean hexa, boolean first_digit) {

  if (hexa) {
    move_to_hexa (p, first_digit);
  } else {
    move_to_ascii (p);
  }
}


static void display_data (unsigned short p, boolean hexa, byte b) {

  move_to (p, hexa, true);

  if (hexa) {
    cprintf ("%02X ", b);
  } else {
    if ( (b > 0x1F) && (b < 0x7F) )
      cprintf("%c", b);
    else
      cprintf("%s", unprintable);
  }
}

static void display_page_num (void) {

  gotoxy (2, 3);
  cprintf ("%6d / %d", current_page, last_page);
}

static void display_pos (void) {
  gotoxy (3, 3);
  cprintf ("Pos: 0x%012lX", file_pos(current_page, pos));
}

static void display_page (void) {
  int i;

  for (i = 0; i < current_page_size; i ++) {
    display_data (i, true,  tab[i]);
    display_data (i, false, tab[i]);
  }
  for (i = current_page_size; i < MU_PAGE_SIZE; i ++) {
    move_to (i, true,  true);  cprintf ("  ");
    move_to (i, false,  true); cprintf (" ");
  }
  display_page_num ();

}

static void modify_page_hex (char car) {

  if (!can_modify) return;

  if (first_hexa)
    tab[pos] = tab[pos] % 16
             + ((car >= '0' && car <= '9') ? (car - '0') * 16 : (car - 'A' + 10) * 16);
  else
    tab[pos] = (tab[pos] / 16) * 16
            + ((car >= '0' && car <= '9') ? (car - '0') : (car - 'A' + 10));

  highvideo ();
  display_data (pos, true,  tab[pos]);
  display_data (pos, false, tab[pos]);
  lowvideo();

  if (!first_hexa) {
    if (pos < (current_page_size - 1) ) pos ++;
  }

  first_hexa = ! first_hexa;

  modified = true;
}

static void modify_page_asc (char car) {

  if (!can_modify) return;

  tab[pos] = car;

  highvideo();
  display_data (pos, true,  tab[pos]);
  display_data (pos, false, tab[pos]);
  lowvideo();

  if (pos < (current_page_size - 1) ) pos ++;

  modified = true;
}

static void write_modif (void) {

  if (! modified) return;

  mvprint (22, 10, "Modif. exists !!! Write (y/n) ? ");

  if (!confirm('Y', 'N')) {
    read_file ();
    display_page ();
  } else {
    write_file ();
  }

  mvprint (22, 10, "                                ");

  modified = false;
  display_page ();
}

/* USER INPUT ************************************************************/

static char get_asc (void) {
  char c;

  c = read_char();
  if (to_erase) {
    mvprint (22, 10, "                          ");
    to_erase = false;
  }
  return (c);
}

static boolean confirm (char yes, char no) {
  char car;

  yes = (char)toupper((int)yes);
  no = (char)toupper((int)no);
  do {
    car = (char)toupper((int)get_asc());
  } while ( (car != yes) && (car != no) && ((int)car != 20) );
  return (car == yes);
}

/* PAGE & FIND ****************************************************************/

#define STR_PAGE_LEN 11

static void goto_page (void) {
  char str[STR_PAGE_LEN], car;
  int i, ii;

  write_modif ();

  for (ii = 0; ii < STR_PAGE_LEN - 1; ii ++) str[ii] = ' ';
  str[STR_PAGE_LEN - 1] = '\0';

  mvprint (22, 10, "Enter page number (or Esc Clear, or Esc Abort) : ");

  i = 0;

  for (; ;) {
    gotoxy (22, 59 + i);
    car = get_asc();
    if (car == 13) {
      break;
    } else if (car == 20) {
      i = 0;
      break;
    } else if (car == 21) {
      i = 0;
      for (ii = 0; ii < STR_PAGE_LEN - 1; ii ++) str[ii] = ' ';
    } else if (car == 8) {
        if (i > 0) {
          i --;
          str[i] = ' ';
        }
    } else if ( (car >= '0') && (car <= '9') ) {
      if (i != 10) {
        str[i] = car;
        i ++;
      }
    }

    mvprint (22, 59, str);
  }

  mvprint (22, 10, "                                                           ");

  if (i == 0) return;

  str[i] = '\0';
  i = atoi (str);

  if ( (i >= 1) && ((unsigned int)i <= last_page) ) {
    current_page = i;
    new_page ();

    pos = 0;
    first_hexa = true;

    read_file ();
    display_page ();
  } else {
    mvprint (22, 10, "No such page !!!");
    to_erase = true;
  }

}

static int to_val (char car) {

  if ( (car >= 'A') && (car <= 'F') ) {
    return ((int) (car - 'A') + 10);
  } else if ( (car >= '0') && (car <= '9') ) {
    return ((int) (car - '0'));
  } else {
    return (-1);
  }
}

#define STR_FIND_LEN 60

static byte read_byte (long file_index) {

  if (file_index % MU_PAGE_SIZE == 0) {
    current_page = file_index / MU_PAGE_SIZE + 1;
    new_page ();
    read_file ();
    display_page_num ();
  }
  pos = file_index % MU_PAGE_SIZE;
  return (tab[pos]);
}

static void find_loop (byte *template, unsigned length) {
  /* Previous position if not found */
  unsigned int page_sav;
  unsigned short pos_sav;

  /* Pos_file is the index in file (0 .. size-1) of read_str[0] */
  long pos_file, file_index;
  byte read_str[(STR_FIND_LEN / 3) * 2];
  int curi;
  int i;

  /* Check length */
  pos_file = file_pos (current_page, pos);
  if ((long long)pos_file + (long long)length > file_size) {
    mvprint (22, 10, "Not found !!!");
    to_erase = true;
    beep (3, 3000);
    return;
  }

  mvprint (22, 10, "Searching ...");

  /* Save current position */
  page_sav = current_page;
  pos_sav = pos;

  /* Read first bloc */
  lseek (file_des, pos_file, SEEK_SET);
  for (i = 0, file_index = pos_file; (unsigned)i < length; i ++, file_index ++) {
    read_str[i] = read_byte(file_index);
    read_str[length + i] = read_str[i];
  }
  curi = 0;

  for (;;) {
    if (memcmp (&read_str[curi], template, length) == 0) {
      /* Beginning of found string */
      pos = pos_file % MU_PAGE_SIZE;
      current_page = pos_file / MU_PAGE_SIZE + 1;
      pos_sav = pos;
      page_sav = current_page;

      /* Show found string */
      new_page ();
      read_file ();
      display_page ();
      display_pos ();

      /* Restore file index */
      lseek (file_des, file_index, SEEK_SET);

      mvprint (22, 10, "Next/Quit ?     ");

      /* To move to found data */
      move_to (pos, hexa_side, true);
      beep (1, 1000);

      if (!confirm('N', 'Q')) {
        mvprint (22, 10, "                 ");
        return;
      }
      mvprint (22, 10, "Searching ...");
      display_page_num ();
    }

    /* Check length */
    if ((long long)file_index >= file_size) {
      current_page = page_sav;
      pos = pos_sav;
      new_page ();
      read_file ();
      display_page ();
      mvprint (22, 10, "Not found !!!");
      to_erase = true;
      beep (3, 3000);
      return;
    }

    /* Next byte */
    read_str[curi] = read_byte (file_index);
    read_str[length + curi] = read_str[curi];

    /* Shift */
    file_index ++;
    pos_file ++;
    curi ++;
    if ((unsigned)curi == length) curi = 0;

  }

}

static unsigned length;
static byte strf[STR_FIND_LEN/3];

static void find_seq (void) {
  char stra[STR_FIND_LEN+1], strh[STR_FIND_LEN+1];
  char str[3];
  boolean out, abort, part_last;
  char car;
  byte b;
  int i, j, val, valf, l, ii;
  int llength;

  /* Save */
  write_modif();

  /* Init */
  for (ii = 0; ii < STR_FIND_LEN; ii ++) {
    stra[ii] = ' ';
    strh[ii] = ' ';
  }
  stra[STR_FIND_LEN] = '\0';
  strh[STR_FIND_LEN] = '\0';

  /* Init from previous search */
  llength = length;
  for  (ii = 0; ii < llength; ii ++) {
      b = strf[ii];
      if ( (b >= 0x1F) && (b < 0x7F) ) {
          stra[3 * ii + 1] = (char) b;
      } else {
          stra[3 * ii + 1] = '.';
      }
      sprintf (str, "%02X", (int)b);
      strh[3 * ii]     = str[0];
      strh[3 * ii + 1] = str[1];
  }

  j = 0;
  i = llength * 3; l = llength * 3;

  mvprint (22, 5, "Enter string, TAB for ascii<->hexa, Esc C to clear, or Esc A to abort :");
  mvprint (23, 3, "ASCII");
  mvprint (24, 3, "HEXA ");

  part_last = false;
  out = false;
  abort = false;

  while (! out) {
    mvprint (23, 10, stra);
    mvprint (24, 10, strh);

    if (!hexa_side) {
      gotoxy (23, 10 + i + 1);
    } else {
      gotoxy (24, 10 + i + j);
    }

    car = get_asc ();
    switch (car) {
      case 13:
        /* return */
        if (!part_last) {
          out = true;
          abort = (l == 0);
        }
      break;

      case 9:
      case 1:
      case 3:
        /* Ascii <-> hexa */
        if (part_last) {
          stra[i+1] = ' ';
          strh[i]   = ' ';
          strh[i+1] = ' ';
          part_last = false;
        }
        hexa_side = ! hexa_side;
        j = 0;
      break;

      case 20:
        /* Esc A */
        out = true;
        abort = true;
        if (part_last) {
          stra[i+1] = ' ';
          strh[i]   = ' ';
          strh[i+1] = ' ';
        }
      break;

      case 21:
        /* Esc C */
        part_last = 0;
        i = 0; j = 0; l = 0; llength = 0;
        for (ii = 0; ii < STR_FIND_LEN; ii ++) {
          stra[ii] = ' ';
          strh[ii] = ' ';
        }

      break;

      case 8:
      /* Back space */
        if (!hexa_side) {
          if ( (i > 0) && (i == l) ) {
            i -= 3;
            l -= 3;
            stra[i+1] = ' ';
            strh[i]   = ' ';
            strh[i+1] = ' ';
          }
        } else {
          if (j == 0) {
            if ( (i > 0) && (i == l) ) {
              i -= 3;
              l -= 3;
              j = 1;
              stra[i+1] = ' ';
              strh[i+1] = ' ';
              part_last = true;
            }
          } else {
            if (part_last) {
              j = 0;
              strh[i] = ' ';
              part_last = false;
            }
          }
        }
      break;

      case 2:
        /* Right */
        if (!hexa_side) {
          if (i != l) {
            i += 3;
          }
        } else {
          if (j == 1) {
            if (i != l) {
              i += 3;
              j = 0;
            }
          } else {
            if (strh[i] != ' ') j = 1;
          }
        }
      break;

      case 4:
        /* Left */
        if (!hexa_side) {
          if (i != 0) {
            i -= 3;
          }
        } else {
          if (j == 0) {
            if (i != 0) {
              if (part_last) {
                str[i+1]  = ' ';
                strh[i]   = ' ';
                strh[i+1] = ' ';
                part_last = false;
              }
              i -= 3;
              j = 1;
            }
          } else {
            j = !j;
          }
        }
      break;

      default:
        if ( (car >= 0x1F) && (car < 0x7F) ) {
          if (i != STR_FIND_LEN) {
            if (!hexa_side) {
              stra[i + 1] = car;
              sprintf (str, "%02X", (int)car);
              strh[i]   = str[0];
              strh[i+1] = str[1];
              i += 3;
            } else {
              if ( ((car >= '0') && (car <= '9')) ||
                   ((car >= 'a') && (car <= 'z')) || ((car >= 'A') && (car <= 'Z')) ) {
                if ( (car >= 'a') && (car <= 'z') ) car = 'A' + car - 'a';
                val = to_val (car);
                if (val != -1) {
                  if (j == 0) {
                    strh[i] = car;
                    j = 1;
                    part_last = (strh[i + 1] == ' ');
                    if (!part_last) {
                      valf = to_val (strh[i + 1]);
                      if (valf == -1) ABORT("String conv 1st");
                      val = val * 0x10 + valf;
                      if ( (val >= 0x7F) || (val <= 0x1F) ) {
                        stra[i + 1] = '.';
                      } else {
                        stra[i + 1] = (char) val;
                      }
                      strh[i + 1]=car;
                    }
                  } else {
                    valf = to_val (strh[i]);
                    if (valf == -1) ABORT("String conv 2nd");
                    val = valf * 0x10 + val;
                    if ( (val >= 0x7F) || (val <= 0x1F) ) {
                      stra[i + 1] = '.';
                    } else {
                      stra[i + 1] = (char) val;
                    }
                    strh[i + 1] = car;
                    i += 3;
                    j = 0;
                    part_last = false;
                  }
                }
              }
            }
          }
        }
        if (i > l) l = i;
      break;

    }
  }

  mvprint (22, 5, "                                                                       ");

  llength = l / 3;
  if (!abort) {
    /* Save for search */
    length = llength;
    for (i = 0; (unsigned)i < length; i ++) {
      strf[i] = (byte) to_val (strh[3 * i]) * 0x10 + to_val (strh[3 * i + 1]);
    }

    /* Search until no more or user quit */
    find_loop (strf, length);

    read_file ();
    display_page ();

  }

  for (i = 0; i < STR_FIND_LEN; i ++) stra[i] = ' ';
  mvprint (23, 10, stra);
  mvprint (24, 10, stra);
  mvprint (23, 3, "     ");
  mvprint (24, 3, "     ");
}


/*************************************************************************/

int main (int argc, char *argv[]) {
char car;
char *pchar;
boolean done;

  if (argc == 2) {
    file_name = argv[1];
    can_modify = true;
  } else if ( (argc == 3) && (strcmp(argv[1], "-r") == 0) ) {
    file_name = argv[2];
    can_modify = false;
  } else {
    usage ();
    exit(0);
  }

  pchar=getenv("MU_UNPRINTABLE");
  if (pchar && (strlen(pchar) == 1) ) unprintable=pchar;

  open_file ();

  open_keybd ();
  clrscr ();
  lowvideo ();


  current_page = 1;
  new_page ();
  pos = 0;
  first_hexa = true;
  hexa_side = true;
  modified = false;
  to_erase = false;

  title ();

  read_file ();
  display_page ();
  display_pos();


  done = false;
  while (!done) {

    /* Move */
    move_to (pos, hexa_side, first_hexa);

    car = get_asc();

    if ( (hexa_side) && (car >= 'a' && car <= 'f') ) car = 'A' + car - 'a';

    if ( (hexa_side) && ((car >= 'A' && car <= 'F') || (car >= '0' && car <= '9')) ) {
      modify_page_hex (car);

    } else if ( (!hexa_side) && (car >= 0x1F) ) {
      modify_page_asc (car);

    } else {
      switch (car) {
        case 9 : /* tab */
          hexa_side = !hexa_side;
        break;
        case 6 : /* page down */
          page_down();
        break;
        case 5 : /* page up */
          page_up();
        break;
        case 1 : /* up */
          movement (-1,  0);
        break;
        case 2 : /* right */
          movement ( 0,  1);
        break;
        case 3 : /* down */
          movement ( 1,  0);
        break;
        case  4 : /* left */
          movement ( 0, -1);
        break;
        case 10 : /* home */
          g_first_page ();
        break;
        case 11 : /* end */
          g_last_page ();
        break;
        case 15 : /* find */
          find_seq ();
        break;
        case 16 : /* goto page */
          goto_page ();
        break;
        case 17 : /* save modif */
          write_modif();
          display_page ();
        break;
        case 18 : /* exit */
          write_modif();
          done = true;
        break;
        case 19 : /* end page */
          end_mvt ();
        break;
        case 20 : /* begin page */
          home_mvt ();
        break;
        case 22 : /* quit */
          done = true;
        break;
        case 23 : /* undo modif */
          modified = false;
          read_file ();
          display_page ();
        break;
      } /* switch Special code */

    } /* if Special car */

    /* Display current_pos */
    display_pos();

  } /* for */

  close (file_des);

  clrscr ();

  close_keybd ();

  exit (0);
}
