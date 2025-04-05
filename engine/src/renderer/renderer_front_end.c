#include "renderer/renderer_front_end.h"

#include "renderer/render_types.h"
#include "renderer/renderer.h"
#include <stdlib.h>

void renderer_frontend_init(RendererFrontEnd* frontend){
    frontend->backend = malloc(sizeof(Renderer));

    // frontend->init = renderer_init;
    // frontend->draw = renderer_draw;
    // frontend->destroy = renderer_shutdown;
}

void renderer_frontend_destroy(RendererFrontEnd* frontend){

}