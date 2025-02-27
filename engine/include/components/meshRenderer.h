#ifndef MESH_RENDERER_COMPONENT_H
#define MESH_RENDERER_COMPONENT_H

#include "engine_defines.h"
#include "meshTypes.h"
#include "renderer/render_types.h"
#include "assets/asset_types.h"
#include "ecs/ecs_types.h"

typedef struct MeshRenderer_Component_t{
    struct {
        VkBuffer vertexBuffer;
        VkDeviceMemory vertexBufferMemory;
    
        VkBuffer indexBuffer;
        VkDeviceMemory indexBufferMemory;
    }renderContext;
    
    u32 indicesCount;
}MeshRenderer;


API void createMeshRenderer(Model* model, Renderer* renderer, MeshRenderer* out);
API void destroyMeshRenderer(Renderer* renderer, MeshRenderer* meshRenderer);
#endif //MESH_RENDERER_COMPONENT_H