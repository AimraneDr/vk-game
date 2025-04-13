#include "components/UI/uiComponents.h"

#include "meshTypes.h"



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

UI_Element ui_create_container(UI_Style style){
    
    UI_Element new = ui_create_element(style, UI_ELEMENT_TYPE_CONTAINER);

    new.meshdata.verticesCount = verticesCount;
    new.meshdata.vertices = malloc(sizeof(UI_Vertex) * verticesCount);
    memcpy(new.meshdata.vertices, ui_vertices, sizeof(UI_Vertex) * verticesCount);
    new.meshdata.indicesCount = indicesCount;
    new.meshdata.indices = malloc(sizeof(u32) * indicesCount);
    memcpy(new.meshdata.indices, ui_indices, sizeof(u32) * indicesCount);
    new.state_is_dirty = true;

    return new;
}