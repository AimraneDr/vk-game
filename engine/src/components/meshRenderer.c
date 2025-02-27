#include "components/meshRenderer.h"

#include "assets/asset_manager.h"
#include "renderer/details/vertexBuffer.h"
#include "renderer/details/indexBuffer.h"

#include <math/mat.h>

void createMeshRenderer(Model* model, Renderer* renderer, MeshRenderer* out){
    // create buffers
    out->indicesCount = model->index_count;
    createVertexBuffer(
        renderer->gpu,
        renderer->device,
        renderer->queue.graphics,
        renderer->commandPool,
        model->vertex_count,
        model->vertices,
        sizeof(Vertex),
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

void destroyMeshRenderer(Renderer* renderer, MeshRenderer* meshRenderer){
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