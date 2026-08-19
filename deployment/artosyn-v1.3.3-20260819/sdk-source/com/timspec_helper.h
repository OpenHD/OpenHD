#ifndef __TIMSPEC_HELPER_H__
#define __TIMSPEC_HELPER_H__

#include <stdint.h>
#include <time.h>

int     get_timespec_from_now(struct timespec* pspec, int msecond);
int64_t get_timespec_diff_in_ms(struct timespec* spec_start, struct timespec* spec_end);
#endif
