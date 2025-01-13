#ifndef RENDERER_H
#define RENDERER_H

#include "engine/platform/platform_types.h"
#include "render_types.h"

Result renderer_init(Renderer* r, PlatformState* p);
Result renderer_shutdown(Renderer* r);


#endif //RENDERER_H