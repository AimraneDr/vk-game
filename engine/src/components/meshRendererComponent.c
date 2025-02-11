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
        renderer->physicalDevice,
        renderer->logicalDevice,
        renderer->graphicsQueue,
        renderer->commandPool,
        model->vertex_count,
        model->vertices,
        &out->renderContext.vertexBuffer,
        &out->renderContext.vertexBufferMemory
    );
    createIndexBuffer(
        renderer->physicalDevice,
        renderer->logicalDevice,
        renderer->graphicsQueue,
        renderer->commandPool,
        model->index_count,
        model->indices,
        &out->renderContext.indexBuffer,
        &out->renderContext.indexBufferMemory
    );
}

void destroyMeshRenderer(Renderer* renderer, MeshRenderer_Component* meshRenderer){
    vkDeviceWaitIdle(renderer->logicalDevice);
    meshRenderer->indicesCount = 0;
    //destroy buffers
    destroyIndexBuffer(
        renderer->logicalDevice, 
        meshRenderer->renderContext.indexBuffer,
        meshRenderer->renderContext.indexBufferMemory
    );
    destroyVertexBuffer(
        renderer->logicalDevice, 
        meshRenderer->renderContext.vertexBuffer,
        meshRenderer->renderContext.vertexBufferMemory
    );
}