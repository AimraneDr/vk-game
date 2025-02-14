#ifndef RENDERER_VULKAN_BACKEND_H
#define RENDERER_VULKAN_BACKEND_H

#include "./vulkan_types.h"
#include "platform/platform_types.h"
#include "renderer/render_types.h"
#include "components/camera.h"
#include "components/meshRenderer.h"
#include "components/UI/ui_types.h"

void vulkan_renderer_init(RendererInitConfig config, RenderState* r, PlatformState* p);
void vulkan_renderer_draw(
    Camera* camera, 
    RenderState* r, PlatformState* p, 
    MeshRenderer* meshRenderers, f64 deltatime,
    UI_Manager* uiManager
);
void vulkan_renderer_shutdown(RenderState* r);


#endif //RENDERER_VULKAN_BACKEND_H