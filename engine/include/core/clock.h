#ifndef CLOCK_H
#define CLOCK_H

#include "engine_defines.h"
#include "data_types.h"

typedef struct Time_t {
    u32 hours;      // 0-23
    u32 minutes;    // 0-59
    u32 seconds;    // 0-59
    u32 milliseconds; // 0-999
} Time;

typedef struct Clock_t{
    f64 totalTime;
    f64 deltaTime;
    f64 lastTick;
}Clock;

API void clock_start(Clock* c);
API void clock_tick(Clock* c);
API Time clock_get_system_time(void);
#endif //CLOCK_H