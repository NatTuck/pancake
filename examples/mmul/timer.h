#ifndef TIMER_H
#define TIMER_H

#include <sys/time.h>

typedef struct timeval timer;

timer* timer_alloc();
void timer_reset(timer*);
double timer_read(timer*);
void timer_free(timer*);

#endif
