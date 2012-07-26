void clrscr (void);

void gotoxy (int x, int y);

void open_keybd (void);

void close_keybd (void);

#define cprintf printf

void highvideo (void);
void lowvideo (void);

long long filelength (int fd);

char read_char (void);

void beep (unsigned char nb_beeps, unsigned int frequency);

