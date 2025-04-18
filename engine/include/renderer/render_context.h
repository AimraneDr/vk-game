#pragma once

#include "renderer/render_types.h"
#include "platform/platform_types.h"

RendererContext* getRendererContext();
RendererContext* createRendererContext(PlatformState* state);
void destroyRendererContext();

/// @brief tell the validation layer debugger that a new frame is being rendered
void renderContext_signalNewFrameForDebugger();