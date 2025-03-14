#include "components/UI/uiComponents.h"

UI_Element ui_createElement(UI_Style s){
    UI_Element new = {0};
    new.style = s;
    new.hovered = false;
    return new;
}

void ui_destroyElement(UI_Element* e, Renderer* r){
    //free mem if allocated
    if(e->renderer.indicesCount != 0){
        vkDeviceWaitIdle(r->context->device);
        e->renderer.indicesCount = 0;
        if(e->renderer.vertexBuffer != 0) vkDestroyBuffer(r->context->device, e->renderer.vertexBuffer, 0);
        if(e->renderer.vertexBufferMemory != 0) vkFreeMemory(r->context->device, e->renderer.vertexBufferMemory, 0);
        if(e->renderer.indexBuffer != 0) vkDestroyBuffer(r->context->device, e->renderer.indexBuffer, 0);
        if(e->renderer.indexBufferMemory != 0) vkFreeMemory(r->context->device, e->renderer.indexBufferMemory, 0);
    }
}