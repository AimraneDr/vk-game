#ifndef MESH_RENDERER_COMPONENT_H
#define MESH_RENDERER_COMPONENT_H

#include "engine_defines.h"
#include "meshTypes.h"
#include "renderer/render_types.h"
#include "assets/asset_types.h"
#include "ecs/ecs_types.h"

#include <math/mathTypes.h>

typedef struct MeshRenderer_Component_t{
    //TODO: change to a ref to material (owned by assets manager)
    Material material;
    void* data;
}MeshRenderer;


API void createMeshRenderer(const char* asset_name, MeshRenderer* out);
API void destroyMeshRenderer(MeshRenderer* mesh);
#endif //MESH_RENDERER_COMPONENT_H