typedef enum {start_command, kill_command} request_kind_list;

typedef unsigned int command_number;
#define MAX_TEXT_LG 512
#define MAX_ENV_LG  256
#define MAX_DIR_LG  256

typedef struct {
  command_number number;
  char command_text[MAX_TEXT_LG];
  char environ_variables[MAX_ENV_LG];
  char currend_dir[MAX_DIR_LG];
  char output_flow[MAX_DIR_LG];
  char error_flow[MAX_DIR_LG];
  unsigned char append_output;
  unsigned char append_error;
} start_request_t;

typedef struct {
  command_number number;
  int    signal_number;
} kill_request_t;

typedef union {
  start_request_t start_request;
  kill_request_t  kill_request;
} request_u;

typedef struct {
  request_kind_list kind;
#define start_req   request.start_request
#define kill_req    request.kill_request
  request_u         request;
} request_message_t;

typedef struct {
  command_number number;
  int    exit_code;
} report_message_t;


