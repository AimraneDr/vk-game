#include "renderer/renderer.h"


void renderer_init(RendererInitConfig config, RenderState* r, PlatformState* p){
    switch(config.backend){
        case RENDERER_BACKEND_VULKAN:

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