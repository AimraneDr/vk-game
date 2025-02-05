#include "core/clock.h"

#include "platform/platform.h"

void clock_start(Clock* c){
    c->lastTick = platform_get_time();
    c->totalTime = 0;
    c->deltaTime = 0;
}

void clock_tick(Clock* c) {
    f64 currentTick = platform_get_time();
    c->deltaTime = currentTick - c->lastTick;
    c->totalTime += c->deltaTime;
    c->lastTick = currentTick;
}
