#ifndef PLATFORM_H
#define PLATFORM_H

#include "data_types.h"
#include "platform/platform_types.h"

//Event Context Types
typedef struct EventContextWindowResize_t{
    u32 width,height;
}EventContextWindowResize;

void window_init(PlatformInitConfig info, PlatformState* out_state);
void window_destroy(PlatformState* state);

void window_PullEvents(PlatformState* state);

/// @return time in seconds
f64 platform_get_time(void);
void platform_sleep(f64 seconds);

#endif //PLATFORM_H