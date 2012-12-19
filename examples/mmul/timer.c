
#include <stdlib.h>

#include "timer.h"

timer*
timer_alloc()
{
    timer* tt = (timer*) malloc(sizeof(timer));
    timer_reset(tt);
    return tt;
}

void 
timer_reset(timer* tt)
{
    struct timeval* tv = (struct timeval*)tt;
    gettimeofday(tv, 0);
}

double
timer_read(timer* tt)
{
    const double MM = 1000000.0;

    struct timeval* tv0 = (struct timeval*)tt;
    struct timeval* tv1 = (struct timeval*)alloca(sizeof(struct timeval));
    gettimeofday(tv1, 0);

    double s0 = tv0->tv_sec + ((double)tv0->tv_usec) / MM;
    double s1 = tv1->tv_sec + ((double)tv1->tv_usec) / MM;

    return s1 - s0;
}

void 
timer_free(timer* tt)
{
    free(tt);
}
