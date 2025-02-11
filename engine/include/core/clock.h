#ifndef CLOCK_H
#define CLOCK_H

#include "engine_defines.h"
#include "data_types.h"

typedef struct Clock_t{
    f64 totalTime;
    f64 deltaTime;
    f64 lastTick;
}Clock;

API void clock_start(Clock* c);
API void clock_tick(Clock* c);

#endif //CLOCK_H