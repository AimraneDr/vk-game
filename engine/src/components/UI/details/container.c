#include "components/UI/uiComponents.h"

#include "meshTypes.h"

#include "renderer/details/vertexBuffer.h"
#include "renderer/details/indexBuffer.h"


const u16 verticesCount = 4;
static const UI_Vertex ui_vertices[] = {
    {{ .x =  1.f, .y =  1.f}, { .x = 1.f, .y = 1.f}},
    {{ .x = 0.f, .y =  1.f}, { .x = 0.f, .y = 1.f}},
    {{ .x = 0.f, .y = 0.f}, { .x = 0.f, .y = 0.f}},
    {{ .x =  1.f, .y = 0.f}, { .x = 1.f, .y = 0.f}}
};

const u16 indicesCount = 6;
static const u32 ui_indices[] = {
    0,2,1, 2,0,3
};

UI_Element ui_create_container(UI_Style style, Renderer* r){
    
    UI_Element new = ui_create_element(style);

    new.renderer.indicesCount = indicesCount;

    //TODO: reference a quad meshData instead
    createVertexBuffer(
        r->context->gpu,
        r->context->device,
        r->context->queue.graphics,
        r->context->commandPool,
        verticesCount, (void*)ui_vertices, sizeof(UI_Vertex),
        &new.renderer.vertexBuffer,
        &new.renderer.vertexBufferMemory
    );
    createIndexBuffer(
        r->context->gpu,
        r->context->device,
        r->context->queue.graphics,
        r->context->commandPool,
        indicesCount, ui_indices,
        &new.renderer.indexBuffer,
        &new.renderer.indexBufferMemory
    );
    return new;
}