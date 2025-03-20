#include "renderer/meshData.h"

#include "core/memsys.h"
#include "renderer/render_context.h"
#include "renderer/details/vertexBuffer.h"
#include "renderer/details/indexBuffer.h"

void createMeshData(u64 vertices_count, Vertex* vertices, u64 indices_count, u32* indices, MeshData** out){
    RendererContext* rc = getRendererContext();
    MeshData* meshDara = memsys_alloc(sizeof(MeshData), MEM_TYPE_GEOMETRY);
    
    // create buffers
    meshDara->indicesCount = indices_count;
    createVertexBuffer(
        rc->gpu,
        rc->device,
        rc->queue.graphics,
        rc->commandPool,
        vertices_count,
        vertices,
        sizeof(Vertex),
        &meshDara->vertexBuffer,
        &meshDara->vertexBufferMemory
    );
    createIndexBuffer(
        rc->gpu,
        rc->device,
        rc->queue.graphics,
        rc->commandPool,
        indices_count,
        indices,
        &meshDara->indexBuffer,
        &meshDara->indexBufferMemory
    );
    *out = meshDara;
}

void destroyMeshData(MeshData** mesh){
    RendererContext* rc = getRendererContext();
    vkDeviceWaitIdle(rc->device);
    (*mesh)->indicesCount = 0;
    //)destroy buffers
    destroyIndexBuffer(
        rc->device, 
        (*mesh)->indexBuffer,
        (*mesh)->indexBufferMemory
    );
    destroyVertexBuffer(
        rc->device,
        (*mesh)->vertexBuffer,
        (*mesh)->vertexBufferMemory
    );
    memsys_free(*mesh, MEM_TYPE_GEOMETRY);
    *mesh = 0;
}
