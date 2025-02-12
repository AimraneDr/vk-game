#include "components/meshRendererComponent.h"

#include "assets/asset_manager.h"
#include "renderer/details/vertexBuffer.h"
#include "renderer/details/indexBuffer.h"

#include <math/mat.h>

void createMeshRenderer(Model* model, Renderer* renderer, MeshRenderer_Component* out){
    // create buffers
    out->indicesCount = model->index_count;
    out->mat4 = MAT4_IDENTITY;
    createVertexBuffer(
        renderer->gpu,
        renderer->device,
        renderer->queue.graphics,
        renderer->commandPool,
        model->vertex_count,
        model->vertices,
        &out->renderContext.vertexBuffer,
        &out->renderContext.vertexBufferMemory
    );
    createIndexBuffer(
        renderer->gpu,
        renderer->device,
        renderer->queue.graphics,
        renderer->commandPool,
        model->index_count,
        model->indices,
        &out->renderContext.indexBuffer,
        &out->renderContext.indexBufferMemory
    );
}

void destroyMeshRenderer(Renderer* renderer, MeshRenderer_Component* meshRenderer){
    vkDeviceWaitIdle(renderer->device);
    meshRenderer->indicesCount = 0;
    //destroy buffers
    destroyIndexBuffer(
        renderer->device, 
        meshRenderer->renderContext.indexBuffer,
        meshRenderer->renderContext.indexBufferMemory
    );
    destroyVertexBuffer(
        renderer->device, 
        meshRenderer->renderContext.vertexBuffer,
        meshRenderer->renderContext.vertexBufferMemory
    );
}