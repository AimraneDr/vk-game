#ifndef CLOCK_H
#define CLOCK_H

#include "engine/data_types.h"

typedef struct Clock_t{
    f64 totalTime;
    f64 deltaTime;
    f64 lastTick;
}Clock;

void clock_start(Clock* c);
void clock_tick(Clock* c);

#endif //CLOCK_H