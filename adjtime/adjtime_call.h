#include <sys/time.h>


/*
if new_delta is non NULL -> change adjustement
if new_delta is NULL -> do not change adjustement
if old_delta is non NULL -> fill it with current adjustement

returns 0 if OK and -1 if error
*/

extern int adjtime_call (struct timeval *new_delta, struct timeval *old_delta);

