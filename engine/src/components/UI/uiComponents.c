#include "components/UI/uiComponents.h"

#include "core/debugger.h"

#include "components/UI/details/builder.h"

void ui_canvas_padding(Vec4 pad)
{
    ui_builder_canvas_padding_set(pad);
}
void ui_canvas_positon(Vec2 pos)
{
    ui_builder_canvas_positon_set(pos);
}
void ui_canvas_size(Vec2 size)
{
    ui_builder_canvas_size_set(size);
}
void ui_canvas_resolution(u32 pixels_per_point)
{
    ui_builder_canvas_resolution_set(pixels_per_point);
}
void ui_canvas_flush()
{
    ui_builder_canvas_flush();
}


UI_Element ui_create_element(UI_Style s)
{
    UI_Element new = {0};
    new.style = s;
    new.hovered = false;
    return new;
}

void ui_destroy_element(UI_Element *e, Renderer *r)
{
    // free mem if allocated
    if (e->renderer.indicesCount != 0)
    {
        vkDeviceWaitIdle(r->context->device);
        e->renderer.indicesCount = 0;
        if (e->renderer.vertexBuffer != 0)
            vkDestroyBuffer(r->context->device, e->renderer.vertexBuffer, 0);
        if (e->renderer.vertexBufferMemory != 0)
            vkFreeMemory(r->context->device, e->renderer.vertexBufferMemory, 0);
        if (e->renderer.indexBuffer != 0)
            vkDestroyBuffer(r->context->device, e->renderer.indexBuffer, 0);
        if (e->renderer.indexBufferMemory != 0)
            vkFreeMemory(r->context->device, e->renderer.indexBufferMemory, 0);
    }
}