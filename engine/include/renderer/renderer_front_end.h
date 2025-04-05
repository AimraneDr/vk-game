#pragma once

#include "data_types.h"
#include "engine_defines.h"

typedef struct RendererFrontEnd_t{
    void* backend;

    //life time
    bool (*init)();
    bool (*draw)();
    bool (*destroy)();

    bool (*createMesh)();
    bool (*destroyMesh)();
    bool (*createTexture)();
    bool (*destroyTexture)();

}RendererFrontEnd;

void renderer_frontend_init(RendererFrontEnd* frontend);
void renderer_frontend_destroy(RendererFrontEnd* frontend);