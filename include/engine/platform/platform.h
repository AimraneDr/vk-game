#ifndef PLATFORM_H
#define PLATFORM_H

#include "game_types.h"
#include "engine/platform/platform_types.h"


Result display_init(PlatformInitConfig info, PlatformState* out_state);
Result destroy_display(PlatformState* state);

Result display_PullEvents(PlatformState* state);


#endif //PLATFORM_H