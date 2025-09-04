
#include "timing.h"

double timespec_elapsed(const struct timespec *start,
                        const struct timespec *end)
{
    if (!start || !end) {
        return 0.0;
    }
    double secs = (double)(end->tv_sec - start->tv_sec);
    double nsecs = (double)(end->tv_nsec - start->tv_nsec) / 1e9;
    return secs + nsecs;
}