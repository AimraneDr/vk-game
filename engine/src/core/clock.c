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

// Get current system time
Time clock_get_system_time(void) {
    Time time = {0};
    
#ifdef _WIN32
    SYSTEMTIME sys_time;
    GetLocalTime(&sys_time);
    
    time.hours = sys_time.wHour;
    time.minutes = sys_time.wMinute;
    time.seconds = sys_time.wSecond;
    time.milliseconds = sys_time.wMilliseconds;
#else
    // For Unix-like systems
    struct timeval tv;
    struct tm* tm_info;
    
    gettimeofday(&tv, NULL);
    tm_info = localtime(&tv.tv_sec);
    
    time.hours = tm_info->tm_hour;
    time.minutes = tm_info->tm_min;
    time.seconds = tm_info->tm_sec;
    time.milliseconds = tv.tv_usec / 1000;
#endif

    return time;
}