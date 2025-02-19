#include "renderer/renderer.h"

#include "core/debugger.h"
#include "core/events.h"

//APIs
#include "renderer/backend/vulkan/vulkan_renderer.h"

void onWindowResize(EventType eType, void *sender, void *listener, EventContext context)
{
    // ((Renderer *)listener)->framebufferResized = true;
}

void renderer_init(RendererInitConfig config, RenderState* r, PlatformState* p){
    switch(config.backend){
        case RENDERER_BACKEND_VULKAN:

            EventListener onResizeListener = {
                .callback = onWindowResize,
                .listener = r};
            subscribe_to_event(EVENT_TYPE_WINDOW_RESIZED, &onResizeListener);

            vulkan_renderer_init(config, r, p);
        break;
        case RENDERER_BACKEND_OPENGL:
        break;
        case RENDERER_BACKEND_DIRECTX:
        break;
        case RENDERER_BACKEND_METAL:
        break;
        default:
            LOG_FATAL("Unknown or no renderer backed is specified");
        break;
    }


}