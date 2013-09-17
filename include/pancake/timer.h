#ifndef CAKE_TIMER_H
#define CAKE_TIMER_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct cake_timer {
    struct timespec ts;
} cake_timer;

#ifdef CAKE_TIMER_FAST_LINK

inline static
void
cake_timer_init()
{
    // do nothing
}

#else

extern FILE* cake_timer_out;

inline static
void
cake_timer_init()
{
    char* out_file;

    if (cake_timer_out == 0) {
        if ((out_file = getenv("CAKE_TIMINGS"))) {
            cake_timer_out = fopen(out_file, "w+");
        }
    }
}

#endif

inline static
void 
cake_timer_reset(cake_timer* tt)
{
    cake_timer_init();
    clock_gettime(CLOCK_MONOTONIC_RAW, &(tt->ts));
}    

inline static
double
cake_timer_ts_to_double(struct timespec* ts)
{
    return ts->tv_sec + ts->tv_nsec * 1.0e-9;
}

inline static
double 
cake_timer_read(cake_timer* tt)
{
    struct timespec ts1;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts1);

    double ss0 = cake_timer_ts_to_double(&(tt->ts));
    double ss1 = cake_timer_ts_to_double(&ts1);

    return ss1 - ss0;
}

#ifndef CAKE_TIMER_FAST_LINK

inline static
void
cake_timer_log(cake_timer* tt, const char* kernel, const char* label)
{
    if (cake_timer_out != 0) {
        double tm = cake_timer_read(tt);
        fprintf(cake_timer_out, "timer_log(%s,%s): %0.06f\n", kernel, label, tm); 
        fflush(cake_timer_out);
    }
}

inline static
void
cake_timer_note(const char* note)
{
    cake_timer_init();

    if (cake_timer_out != 0) {
        fprintf(cake_timer_out, "%s\n", note);
    }
}

#endif

#ifdef __cplusplus
}
#endif

#endif
