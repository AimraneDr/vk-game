#ifndef RENDERER_FRONTEND_H
#define RENDERER_FRONTEND_H

#include "./render_types.h"
#include "platform/platform_types.h"

void renderer_init(RendererInitConfig config, RenderState* r, PlatformState* p);

#endif //RENDERER_FRONTEND_H