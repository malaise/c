/* Display usage message on stderr */
extern void usage (void);

/* Display an error message on stderr and exit */
extern void error (const char *msg, const char *arg)
                  __attribute__ ((noreturn));

/* Parse arguments, exit on error */
extern void parse_args (const int argc, const char *argv[]);

