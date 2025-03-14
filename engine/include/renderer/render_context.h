#pragma once

#include "renderer/render_types.h"
#include "platform/platform_types.h"

RendererContext* getRendererContext();
RendererContext* createRendererContext(PlatformState* state);
void destroyRendererContext();