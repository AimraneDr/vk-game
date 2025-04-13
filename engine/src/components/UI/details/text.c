#include "components/UI/details/text.h"

#include "assets/asset_manager.h"
#include "components/UI/uiComponents.h"
#include <string/str.h>
#include <math/vec2.h>
#include <math/mathUtils.h>

UI_Element ui_create_text(UI_Style style, const char* text){
    UI_Element new = ui_create_element(style, UI_ELEMENT_TYPE_TEXT);
    ui_set_text(&new, text);
    return new;
}

void ui_set_text(UI_Element* elem, const char* text){
    if (elem->properties.text.val && elem->properties.text.len > 0)
    {
        str_free(&elem->properties.text);
    }
    if(!elem->properties.text.val || elem->properties.text.len == 0 || !str_equals_val(elem->properties.text, text)){
        elem->properties.text = str_new(text);
    }

    Font* font = get_asset(elem->style.text.fontName, ASSET_TYPE_FONT)->data;
    f32 scale = elem->style.text.size / MAX((f32)1, font->size);
    Vec2 cursor = vec2_new(elem->style.padding.left, elem->style.padding.top); //TODO: use right for rtl languages
    u32 vertexCount = 4 * elem->properties.text.len;
    UI_Vertex* vertices = malloc(sizeof(Vertex) * vertexCount);
    u32 indexCount = 6 * elem->properties.text.len;
    u32* indices = malloc(sizeof(u32) * indexCount);

    f32 totalWidth = 0.f;
    f32 maxHeight = 0.f;
    for (u32 i = 0; i < elem->properties.text.len; i++) {
        Glyph glyph = font->glyphs[elem->properties.text.val[i]]; // Look up glyph from your parsed data
    
        // Calculate quad position (screen-space)
        f32 x = cursor.x + glyph.offset.x * scale;
        f32 y = cursor.y + glyph.offset.y * scale;
        f32 w = glyph.width * scale;
        f32 h = glyph.height * scale;
        f32 advance = glyph.advance * scale;
    
        // Define quad vertices (positions and UVs)
        //aply font size
        UI_Vertex glyph_vertices[4] = {
            { {x,     y},     {glyph.uv0.u, glyph.uv0.v} },
            { {x + w, y},     {glyph.uv1.u, glyph.uv0.v} },
            { {x + w, y + h}, {glyph.uv1.u, glyph.uv1.v} },
            { {x,     y + h}, {glyph.uv0.u, glyph.uv1.v} }
        };
    
        // Add vertices to a dynamic vertex buffer
        vertices[i * 4 + 0] = glyph_vertices[0];
        vertices[i * 4 + 1] = glyph_vertices[1];
        vertices[i * 4 + 2] = glyph_vertices[2];
        vertices[i * 4 + 3] = glyph_vertices[3];

        //indices
        //0,2,1, 2,0,3
        indices[i * 6 + 0] = i * 4 + 0;
        indices[i * 6 + 1] = i * 4 + 2;
        indices[i * 6 + 2] = i * 4 + 1;
        indices[i * 6 + 3] = i * 4 + 2;
        indices[i * 6 + 4] = i * 4 + 0;
        indices[i * 6 + 5] = i * 4 + 3;

        // Advance cursor
        cursor.x += glyph.advance;
        totalWidth += glyph.advance;
        maxHeight = MAX(maxHeight, y + h);
    }
    elem->style.width = totalWidth;
    elem->style.height = maxHeight;
    elem->state_is_dirty = true;
    elem->meshdata.verticesCount = vertexCount;
    elem->meshdata.vertices = vertices;
    elem->meshdata.indicesCount = indexCount;
    elem->meshdata.indices = indices;
    elem->properties.font = font;
}

void ui_set_text_font(UI_Element* elem, const char* fontName){

}
