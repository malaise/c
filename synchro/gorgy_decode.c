#include <stdio.h>
#include <time.h>

#include "gorgy_decode.h"


int gorgy_decode (char frame[], struct timeval *p_new_time, char *p_precision) {
   struct tm   tms;
   time_t      new_time_secs;

    /* Decode external clock message */
    if (sscanf(frame, "%*4s %d/%d/%d  %d:%d:%d%c\n",
                       &tms.tm_mday, &tms.tm_mon,
                       &tms.tm_year, &tms.tm_hour,
                       &tms.tm_min,  &tms.tm_sec,
                       p_precision) != 7) {
        return (-1);
    }
    tms.tm_year   = (tms.tm_year < 70) ? tms.tm_year + 100 : tms.tm_year;
    tms.tm_mon   -= 1;
    tms.tm_isdst  = 0;
    new_time_secs = mktime(&tms);
    if (new_time_secs == (time_t)-1) {
        return (-1);
    }
    p_new_time->tv_sec = new_time_secs;
    p_new_time->tv_usec = 0;

    return (0);
}

