#include "components/UI/uiComponents.h"

#include "meshTypes.h"

#include "renderer/details/vertexBuffer.h"
#include "renderer/details/indexBuffer.h"


const u16 verticesCount = 4;
static const UI_Vertex ui_vertices[] = {
    {{.5f,.5f}, {1.f, 1.f}},
    {{-.5f,.5f}, {0.f, 1.f}},
    {{-.5f,-.5f}, {0.f, 0.f}},
    {{.5f,-.5f}, {1.f, 0.f}}
};

const u16 indicesCount = 6;
static const u32 ui_indices[] = {
    0,1,2, 2,3,0
};

UI_Element* ui_create_container(UI_Element* parent, Transform2D transform, UI_Style style, Renderer* r){
    
    UI_Element* new = ui_createElement(parent, transform, style);

    new->renderer.indicesCount = indicesCount;

    createVertexBuffer(
        r->gpu,
        r->device,
        r->queue.graphics,
        r->commandPool,
        verticesCount, ui_vertices, sizeof(UI_Vertex),
        &new->renderer.vertexBuffer,
        &new->renderer.vertexBufferMemory
    );
    createIndexBuffer(
        r->gpu,
        r->device,
        r->queue.graphics,
        r->commandPool,
        indicesCount, ui_indices,
        &new->renderer.indexBuffer,
        &new->renderer.indexBufferMemory
    );
    return new;
}