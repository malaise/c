#include "socket.h"

/* Parse arguments, exit on error */
extern void parse_args (const int argc, const char *argv[]);

/* Trace a message if debug is on */
extern void trace (const char *msg, const char *arg);

/* Display usage message on stderr */
extern void usage (void);

/* Display an error message on stderr and exit */
extern void error (const char *msg, const char *arg)
                  __attribute__ ((noreturn));

/* Bind the socket to the IPM lan and port */
extern void bind_socket (soc_token socket);

/* Put message info */
extern void display (const soc_token socket, 
                     const char *message, const int length);

