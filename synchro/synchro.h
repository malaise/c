#include "timeval.h"

#define magic_request_value 0x21212121
#define magic_reply_value   0x12121212

typedef struct {
    unsigned int magic_number;
    timeout_t request_time;
    timeout_t server_time;
} synchro_msg_t;

