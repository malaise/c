
/****************************************************************************/
/****** Preliminary definitions                                        ******/
/****************************************************************************/
typedef enum {start_command, kill_command} request_kind_list;

typedef unsigned int command_number;
#define MAX_TEXT_LG 512               /* Program + arguments               */
#define MAX_ENV_LG  256               /* Environment variables             */
#define MAX_DIR_LG  256               /* Current dir/stdout /stderr        */

/* Start request structure */
typedef struct {
  command_number number;              /* Managed by the client              */
  char command_text[MAX_TEXT_LG];     /* program\0arg1\0arg2\0 ... argb\0\0 */
  char environ_variables[MAX_ENV_LG]; /* var1=val1\0 ........ varn=valn\0\0 */
  char currend_dir[MAX_DIR_LG];       /* path\0                             */
  char output_flow[MAX_DIR_LG];       /* path_to_file\0                     */
  unsigned char append_output;        /* 0 (create) or 1 (append)           */
  char error_flow[MAX_DIR_LG];        /* path_to_file\0                     */
  unsigned char append_error;         /* 0 (create) or 1 (append)           */
} start_request_t;

/* Kill request structure */
typedef struct {
  command_number number;              /* Managed by the client              */
  int    signal_number;               /* Signal num to send                 */
} kill_request_t;

typedef union {
  start_request_t start_request;
  kill_request_t  kill_request;
} request_u;


/****************************************************************************/
/****** Interface definitions                                          ******/
/****************************************************************************/
/* The interface request message client -> forker */
typedef struct {
  request_kind_list kind;
#define start_req   request.start_request
#define kill_req    request.kill_request
  request_u         request;
} request_message_t;

/* The interface report message forker -> client */
typedef struct {
  command_number number;              /* Managed by the client              */
  int    exit_code;                   /* Exit code of the command           */
} report_message_t;


