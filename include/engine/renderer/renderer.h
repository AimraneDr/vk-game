#ifndef RENDERER_H
#define RENDERER_H

#include "engine/platform/platform_types.h"
#include "render_types.h"

Result renderer_init(Renderer* r, PlatformState* p);
void renderer_draw(Renderer* r, PlatformState* p, f64 deltatime);
Result renderer_shutdown(Renderer* r);


#endif //RENDERER_H