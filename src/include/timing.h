

#ifndef TIMING_H
#define TIMING_H

#include <time.h>

double timespec_elapsed(const struct timespec *start,
                        const struct timespec *end);

#endif /* TIMING_H */