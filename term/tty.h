void init_tty (const char *arg, int ctsrts, int echo, int crlf);

void restore_tty (void);

void send_tty (char c);

void read_tty (char *c);

int get_tty_fd(void);

void init_kbd (int kfd);

void restore_kbd (int kfd);

