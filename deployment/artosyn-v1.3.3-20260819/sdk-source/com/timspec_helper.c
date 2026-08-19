#include "timspec_helper.h"
#include <stdint.h>

int get_timespec_from_now(struct timespec* pspec, int msecond)
{
    if (msecond <= 0 || !pspec) {
        return -1;
    }

    uint64_t m64 = msecond;

#ifdef WIN32
    int ret = timespec_get(pspec, TIME_UTC);
#else
    int ret = clock_gettime(CLOCK_REALTIME, pspec);
#endif

    uint64_t tmp = pspec->tv_nsec + m64 * 1000 * 1000;

    uint64_t a = tmp / (1000 * 1000 * 1000);
    uint64_t b = tmp % (1000 * 1000 * 1000);

    pspec->tv_sec += a;
    pspec->tv_nsec = b;

    return 0;
}

int64_t get_timespec_diff_in_ms(struct timespec* spec_start, struct timespec* spec_end)
{
    uint64_t tick_start = spec_start->tv_sec * 1000 + spec_start->tv_nsec / 1000 / 1000;
    uint64_t tick_end   = spec_end->tv_sec * 1000 + spec_end->tv_nsec / 1000 / 1000;

    int64_t res = tick_end - tick_start;

    return res;
}
