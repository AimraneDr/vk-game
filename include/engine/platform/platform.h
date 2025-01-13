#ifndef PLATFORM_H
#define PLATFORM_H

#include "engine/data_types.h"
#include "engine/platform/platform_types.h"


Result window_init(PlatformInitConfig info, PlatformState* out_state);
Result window_destroy(PlatformState* state);

Result window_PullEvents(PlatformState* state);


#endif //PLATFORM_H