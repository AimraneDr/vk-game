#ifndef PLATFORM_H
#define PLATFORM_H

#include "engine/data_types.h"
#include "engine/platform/platform_types.h"
//Event Context Types
typedef struct EventContextWindowResize_t{
    u32 width,height;
}EventContextWindowResize;

Result window_init(PlatformInitConfig info, PlatformState* out_state);
Result window_destroy(PlatformState* state);

void window_PullEvents(PlatformState* state);

/// @return time in seconds
f64 platform_get_time(void);

#endif //PLATFORM_H