/* Clear screen */
void clrscr (void);

/* Move to row, col */
void gotoxy (int x, int y);

/* Open / close keyboard in raw mode */
void open_keybd (void);

void close_keybd (void);

#define cprintf printf

/* Swith to bold */
void highvideo (void);
void lowvideo (void);

/* Length of a opened file */
long long filelength (int fd);

/* Read char */
/* Special values:                               */
/* 01, 02, 03, 04 => Arrow Up/Right/Down/Left    */
/* 05, 06 => Page Up/Down                        */
/* 08 => Backspace                               */
/* 09 => Tab                                     */
/* 10, 11 => Home/End                            */
/* 13 => Return                                  */
/* 19, 20 => End/Begin page                      */
/* 15, 16, 17, 18, 21, 22, 23 => Esc F/P/X/C/Q/U */
/* 0x1B => Esc Esc                               */
char read_char (void);

/* Put a BELL char */
void beep (unsigned char nb_beeps, unsigned int frequency);

