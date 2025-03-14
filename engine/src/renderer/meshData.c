#include "renderer/meshData.h"

#include "renderer/render_context.h"
#include "renderer/details/vertexBuffer.h"
#include "renderer/details/indexBuffer.h"

void createMeshData(Model* model, MeshData* out){
    RendererContext* rc = getRendererContext();
    // create buffers
    out->indicesCount = model->index_count;
    createVertexBuffer(
        rc->gpu,
        rc->device,
        rc->queue.graphics,
        rc->commandPool,
        model->vertex_count,
        model->vertices,
        sizeof(Vertex),
        &out->vertexBuffer,
        &out->vertexBufferMemory
    );
    createIndexBuffer(
        rc->gpu,
        rc->device,
        rc->queue.graphics,
        rc->commandPool,
        model->index_count,
        model->indices,
        &out->indexBuffer,
        &out->indexBufferMemory
    );
}

void destroyMeshData(MeshData* mesh){
    RendererContext* rc = getRendererContext();
    vkDeviceWaitIdle(rc->device);
    mesh->indicesCount = 0;
    //destroy buffers
    destroyIndexBuffer(
        rc->device, 
        mesh->indexBuffer,
        mesh->indexBufferMemory
    );
    destroyVertexBuffer(
        rc->device,
        mesh->vertexBuffer,
        mesh->vertexBufferMemory
    );
}
